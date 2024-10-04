#include "Common.hpp"
#include "Core.h"
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <cstddef>
#include <memory>
#include <nlohmann/json.hpp>
#include <optional>
#include <unordered_map>

using boost::asio::ip::tcp;

class session : public std::enable_shared_from_this<session> {
public:
  session(tcp::socket socket,
          std::unordered_map<size_t, std::weak_ptr<session>> &clients)
      : socket_(std::move(socket)), clients_(clients) {}

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
        clients_[core.get_last_user_id()] = weak_from_this();
        reply = "Registration successful";
      }

      if (reqType == Requests::ViewBalance) {
        reply = core.get_info(j["UserId"]);
      }

      /*if (reqType == Requests::ViewOrders) {
        core.view_orders(j["UserId"]);
        reply = "Orders viewed successfully";
      }*/ //TODO

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

        side == "Buy" ? core.make_order<ESide::EBuy>(j["UserId"], j["Quantity"],
                                                     j["Price"])
                      : core.make_order<ESide::ESell>(
                            j["UserId"], j["Quantity"], j["Price"]);

        reply = "Order placed successfully";
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
  std::unordered_map<size_t, std::weak_ptr<session>> &clients_;
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
    socket_.emplace(io_context_);
    acceptor_.async_accept(*socket_, [this](std::error_code ec) {
      if (!ec) {
        std::make_shared<session>(std::move(socket_), clients_)->start();
      }

      start_accept();
    });
  }

  boost::asio::io_context &io_context_;
  std::optional<tcp::socket> socket_;
  tcp::acceptor acceptor_;
  std::unordered_map<size_t, std::weak_ptr<session>> clients_;
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