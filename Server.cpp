#include "Common.hpp"
#include "Core.h"
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <nlohmann/json.hpp>

using boost::asio::ip::tcp;

class session {
public:
  session(boost::asio::io_context &io_context) : socket_(io_context) {}

  tcp::socket &socket() { return socket_; }

  void start() {
    socket_.async_read_some(
        boost::asio::buffer(data_, max_length),
        boost::bind(&session::handle_read, this,
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred));
  }

  void handle_read(const boost::system::error_code &error,
                   size_t bytes_transferred) {
    if (!error) {
      data_[bytes_transferred] = '\0';

      auto j = nlohmann::json::parse(data_);
      auto reqType = j["ReqType"];
      auto core = Core::get();

      std::string_view reply = "Error! Unknown request type";
      if (reqType == Requests::Registration) {
        core.reg();
        reply = "Registration successful";
      }
      if (reqType == Requests::MakeOrder) {
        auto side = j["Side"];
        if (side != "Buy" && side != "Sell") {
          reply = "Error! Unknown side";
          boost::asio::async_write(
              socket_, boost::asio::buffer(reply.data(), reply.size()),
              boost::bind(&session::handle_write, this,
                          boost::asio::placeholders::error));
          return;
        }

        auto to_notify = side == "Buy"
                             ? core.make_order<ESide::EBuy>(
                                   j["UserId"], j["Quantity"], j["Price"])
                             : core.make_order<ESide::ESell>(
                                   j["UserId"], j["Quantity"], j["Price"]);
        for (const auto &user_id : to_notify) {
          auto &messages = core.get_to_send(user_id);
          for (const auto &message : messages) {
            std::string msg = message.dump();
            boost::asio::async_write(
                socket_, boost::asio::buffer(msg.data(), msg.size()),
                boost::bind(&session::handle_write, this,
                            boost::asio::placeholders::error));
          }
        }
        reply = "Order placed successfully";
      }

      if (reqType == Requests::ViewOrders) {
        core.view_orders(j["UserId"]);
        reply = "Orders viewed successfully";
      }

      boost::asio::async_write(socket_,
                               boost::asio::buffer(reply.data(), reply.size()),
                               boost::bind(&session::handle_write, this,
                                           boost::asio::placeholders::error));
    } else {
      delete this;
    }
  }

  void handle_write(const boost::system::error_code &error) {
    if (!error) {
      socket_.async_read_some(
          boost::asio::buffer(data_, max_length),
          boost::bind(&session::handle_read, this,
                      boost::asio::placeholders::error,
                      boost::asio::placeholders::bytes_transferred));
    } else {
      delete this;
    }
  }

private:
  tcp::socket socket_;
  enum { max_length = 1024 };
  char data_[max_length];
};

class server {
public:
  server(boost::asio::io_context &io_context, short port)
      : io_context_(io_context),
        acceptor_(io_context, tcp::endpoint(tcp::v4(), port)) {
    start_accept();
  }

private:
  void start_accept() {
    session *new_session = new session(io_context_);
    acceptor_.async_accept(new_session->socket(),
                           boost::bind(&server::handle_accept, this,
                                       new_session,
                                       boost::asio::placeholders::error));
  }

  void handle_accept(session *new_session,
                     const boost::system::error_code &error) {
    if (!error) {
      new_session->start();
    } else {
      delete new_session;
    }

    start_accept();
  }

  boost::asio::io_context &io_context_;
  tcp::acceptor acceptor_;
};

int main(int argc, char *argv[]) {
  try {
    if (argc != 2) {
      std::cerr << "Usage: server <port>\n";
      return 1;
    }

    boost::asio::io_context io_context;
    server s(io_context, std::atoi(argv[1]));
    io_context.run();
  } catch (std::exception &e) {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}