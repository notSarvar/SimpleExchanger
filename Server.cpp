#include "Common.hpp"
#include "Core.h"
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <cstddef>
#include <iostream>
#include <memory>
#include <nlohmann/json.hpp>
#include <string>
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
            auto &core = Core::get();
            if (reqType == Requests::Registration) {
              user_id_ = core.reg();
              clients_[user_id_] = weak_from_this();
            }
            on_write("Registration of user" + std::to_string(user_id_) +
                     "successful");
          } else {
            std::cout << "Error: " << ec.message() << std::endl;
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
            auto &core = Core::get();

            std::string log = "Bad request";
            if (reqType == Requests::ViewInfo) {
              log = "ViewInfo for user: " + std::to_string(user_id_) + "\n";
              core.user_info(user_id_);
            }

            if (reqType == Requests::ViewOrders) {
              log = "ViewOreders for user: " + std::to_string(user_id_) + '\n';
              core.view_orders(user_id_);
            }

            if (reqType == Requests::MakeOrder) {
              log = "MakeOrder for user: " + std::to_string(user_id_) + '\n';
              auto side = j["Side"];
              auto &to_notify = side == "Buy"
                                    ? core.make_order<ESide::EBuy>(
                                          user_id_, j["Quantity"], j["Price"])
                                    : core.make_order<ESide::ESell>(
                                          user_id_, j["Quantity"], j["Price"]);
              for (size_t id : to_notify) {
                do_notify(id);
              }
            }

            on_write(std::move(log));
          } else {
            std::cout << "Error: " << ec.message() << std::endl;
          }
        });
  }

  void on_write(std::string log) {
    std::cout << log << std::endl;
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
    std::cout << "Response for user: " << user_id << std::endl;
    nlohmann::json response;
    for (auto &&j : Core::get().get_to_send(user_id)) {
      response += j;
    }
    Core::get().get_to_send(user_id).resize(0);
    std::cout << "Response: " << response << std::endl;
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
      : socket_(io_context),
        acceptor_(io_context, tcp::endpoint(tcp::v4(), port)) {
    std::cout << "Server started on port: " << port << std::endl;
    start_accept();
  }

private:
  void start_accept() {
    acceptor_.async_accept(socket_, [this](std::error_code ec) {
      if (!ec) {
        std::make_shared<session>(std::move(socket_), clients_)->start();
      }

      start_accept();
    });
  }

  tcp::socket socket_;
  tcp::acceptor acceptor_;
  std::unordered_map<size_t, std::weak_ptr<session>> clients_;
};

int main(int argc, char *argv[]) {
  try {
    boost::asio::io_context io_context;
    Core::init();
    server s(io_context, argc > 1 ? std::atoi(argv[1]) : 5555);
    io_context.run();
  } catch (std::exception &e) {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}