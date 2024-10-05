#include "nlohmann/json.hpp"
#include <boost/asio.hpp>
#include <iostream>

using boost::asio::ip::tcp;

class Client {
public:
  Client(boost::asio::io_context &io_context,
         const tcp::resolver::results_type &endpoints)
      : socket_(io_context) {
    do_connect(endpoints);
  }

private:
  void close() {
    socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both);
    socket_.close();
  }

  void do_connect(const tcp::resolver::results_type &endpoints) {
    boost::asio::async_connect(
        socket_, endpoints,
        [this](boost::system::error_code ec, const tcp::endpoint &) {
          if (!ec) {
            auth();
          }
        });
  }

  void do_read() {
    boost::asio::async_read_until(
        socket_, buffer_, '\0', [this](std::error_code ec, size_t) {
          if (!ec) {
            std::istream is(&buffer_);
            std::string line(std::istreambuf_iterator<char>(is), {});
            auto j = nlohmann::json::parse(line);
            std::cout << "Response: " << j.dump() << "\n";
            do_read();
          }
        });
  }

  void auth() {
    std::cout << "Menu:\n";
    std::cout << 1 << ") "
              << "Exit" << '\n';
    std::cout << 2 << ") "
              << "Registration" << '\n';

    int menu_option_num;
    std::cin >> menu_option_num;
    switch (menu_option_num) {
    case 2: {
      do_write("Reg");
      do_read();
      break;
    }
    case 1: {
      close();
      break;
    }
    default: {
      std::cout << "Unknown menu option\n" << std::endl;
      auth();
    }
    }
  }

  void menu() {
    std::cout << "Menu:\n";
    std::cout << 1 << ") "
              << "Exit" << '\n';
    std::cout << 2 << ") "
              << "ViewOrders" << '\n';
    std::cout << 3 << ") "
              << "ViewUserInfo" << '\n';
    std::cout << 4 << ") "
              << "MakeOrder" << '\n';
    int menu_option_num;
    std::cin >> menu_option_num;
    switch (menu_option_num) {
    case 2: {
      do_write("VOrders");
      break;
    }
    case 3: {
      do_write("VInfo");
      break;
    }
    case 4: {
      std::cout << "Enter: Side Quantity Price: ";
      std::string side;
      int q, p;
      std::cin >> side >> q >> p;
      if (q <= 0 || p <= 0 || (side != "Buy" && side != "Sell")) {
        std::cout << "Incorrect data\n";
        menu();
      } else {
        do_write("MOrder", side, q, p);
      }
      break;
    }
    case 1: {
      close();
      break;
    }
    default: {
      std::cout << "Unknown menu option\n" << std::endl;
      menu();
    }
    }
  }

  template <typename... Args>
  void do_write(std::string req_type, std::string side = "", int q = 0,
                int p = 0) {
    nlohmann::json req;
    req["ReqType"] = req_type;
    if (req_type == "MOrder") {
      req["Side"] = side;
      req["Quantity"] = q;
      req["Price"] = p;
    }

    boost::asio::async_write(
        socket_, boost::asio::buffer(req.dump(), req.dump().size() + 1),
        [this](std::error_code ec, size_t) {
          if (!ec) {
            menu();
          }
        });
  }

  tcp::socket socket_;
  boost::asio::streambuf buffer_;
};

int main(int argc, char *argv[]) {
  try {
    boost::asio::io_context io_context;

    tcp::resolver resolver(io_context);

    Client client(io_context, resolver.resolve(
                                  tcp::v4(), (argc > 1 ? argv[1] : "127.0.0.1"),
                                  (argc > 2 ? argv[2] : "5555")));

    std::thread t([&]() { io_context.run(); });
    io_context.run();

    t.join();
  } catch (std::exception &e) {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}