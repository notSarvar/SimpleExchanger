#include "Common.hpp"
#include <boost/asio.hpp>
#include <iostream>
#include <nlohmann/json.hpp>
#include <string>
#include <thread>

using boost::asio::ip::tcp;

class Client {
public:
  Client(boost::asio::io_context &io_context,
         const tcp::resolver::results_type &endpoints)
      : socket_(io_context) {
    do_connect(endpoints);
  }

  void menu() {
    std::cout << "Menu:\n"
                 "1) Make Order\n"
                 "2) View Orders\n"
                 "3) Exit\n"
              << std::endl;

    short menu_option_num;
    std::cin >> menu_option_num;
    switch (menu_option_num) {
    case 1: {
      nlohmann::json order_data;
      std::cout << "Enter side (0 for Buy, 1 for Sell): ";
      int side;
      std::cin >> side;
      order_data["Side"] = side;

      std::cout << "Enter quantity: ";
      int quantity;
      std::cin >> quantity;
      order_data["Quantity"] = quantity;

      std::cout << "Enter price: ";
      int price;
      std::cin >> price;
      order_data["Price"] = price;

      do_write("ReqType", Requests::MakeOrder, "UserId", my_id_, "Data",
               order_data);
      break;
    }
    case 2: {
      do_write("ReqType", Requests::ViewOrders, "UserId", my_id_);
      break;
    }
    case 3: {
      exit(0);
      break;
    }
    default: {
      std::cout << "Unknown menu option\n" << std::endl;
    }
    }
  }

private:
  void do_connect(const tcp::resolver::results_type &endpoints) {
    boost::asio::async_connect(socket_, endpoints,
                               [this](std::error_code ec, tcp::endpoint) {
                                 if (!ec) {
                                   do_read();
                                   do_write("ReqType", Requests::Registration);
                                 }
                               });
  }

  void do_read() {
    boost::asio::async_read_until(
        socket_, buffer_, "\n", [this](std::error_code ec, std::size_t length) {
          if (!ec) {
            std::istream is(&buffer_);
            std::string line;
            std::getline(is, line);
            auto response = nlohmann::json::parse(line);

            if (response.contains("UserId")) {
              my_id_ = response["UserId"];
            }

            std::cout << "Server response: " << line << std::endl;
            menu();
          }
        });
  }

  template <typename... Args> void do_write(const Args &...args) {
    auto request = make_req(std::make_pair(args, args)...);
    boost::asio::async_write(socket_,
                             boost::asio::buffer(request, request.size()),
                             [this](std::error_code ec, size_t) {
                               if (!ec) {
                                 do_read();
                               }
                             });
  }

  template <typename... Args> std::string make_req(const Args &...args) {
    nlohmann::json req;
    auto add = [&req](auto &pair) { req[pair.first] = pair.second; };

    [[maybe_unused]] int dummy[sizeof...(Args)] = {(add(args), 0)...};
    return req.dump() + "\n";
  }

  tcp::socket socket_;
  boost::asio::streambuf buffer_;
  std::string my_id_;
};

int main(int argc, char *argv[]) {
  try {
    boost::asio::io_context io_context;
    tcp::resolver resolver(io_context);

    Client client(io_context, resolver.resolve(
                                  tcp::v4(), (argc > 1 ? argv[1] : "127.0.0.1"),
                                  (argc > 2 ? argv[2] : "12345")));

    std::thread t([&io_context]() { io_context.run(); });
    io_context.run();

    t.join();
  } catch (std::exception &e) {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}