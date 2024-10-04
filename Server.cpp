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

  void start() { first_read(); }

private:
  void first_read() {
    auto self(shared_from_this());
    socket_.async_read_some(
        boost::asio::buffer(data_, max_length),
        [this, self](std::error_code ec, size_t bytes_transferred) {
          if (!ec) {
            data_[bytes_transferred] = '\0';
            auto j = nlohmann::json::parse(data_);
            auto reqType = j["ReqType"];
            auto core = Core::get();

            if (reqType == Requests::Registration) {
              user_id_ = core.reg();
              clients_[user_id_] = weak_from_this();
            }
            on_write();
          }
        });
  }

  void on_read() {
    auto self(shared_from_this());
    socket_.async_read_some(
        boost::asio::buffer(data_, max_length),
        [this, self](std::error_code ec, size_t bytes_transferred) {
          if (!ec) {
            data_[bytes_transferred] = '\0';
            auto j = nlohmann::json::parse(data_);
            auto reqType = j["ReqType"];
            auto core = Core::get();

            if (reqType == Requests::ViewInfo) {
              core.get_info(user_id_);
            }

            if (reqType == Requests::ViewOrders) {
              core.view_orders(user_id_);
            }

            if (reqType == Requests::MakeOrder) {
              auto side = j["Side"];
              auto to_notify =
                  side == "Buy" ? core.make_order<ESide::EBuy>(
                                      j["UserId"], j["Quantity"], j["Price"])
                                : core.make_order<ESide::ESell>(
                                      j["UserId"], j["Quantity"], j["Price"]);
              for (size_t id : to_notify) {
                do_notify(id);
              }
            }

            on_write();
          }
        });
  }

  void on_write() {
    auto self(shared_from_this());
    auto response = make_response(user_id_);
    auto str = response.dump();
    boost::asio::async_write(
        socket_, boost::asio::buffer(std::move(str) + '\0'),
        [this, self, response = std::move(response)](std::error_code ec,
                                                     size_t /*length*/) {
          if (!ec) {
            on_read();
          } else {
            Core::get().get_to_send(user_id_).push_back(response);
          }
        });
  }

  void do_notify(size_t user_id) {
    if (clients_[user_id].expired()) {
      return;
    }
    auto session = clients_[user_id].lock();
    auto response = make_response(user_id);
    auto str = response.dump();
    boost::asio::async_write(
        session->socket_, boost::asio::buffer(std::move(str) + '\0'),
        [this, session, response = std::move(response),
         user_id](std::error_code ec, size_t /*length*/) {
          if (ec) {
            Core::get().get_to_send(user_id).push_back(response);
            do_notify(user_id);
          }
        });
  }

  nlohmann::json make_response(size_t user_id) {
    nlohmann::json response;
    for (auto &&j : Core::get().get_to_send(user_id)) {
      response += j;
    }
    Core::get().get_to_send(user_id).resize(0);
    return response;
  }

private:
  tcp::socket socket_;
  enum { max_length = 1024 };
  char data_[max_length];
  size_t user_id_;
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