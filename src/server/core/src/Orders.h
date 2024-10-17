#include <Common.hpp>
#include <json.hpp>

#include <queue>

struct Order {
  size_t order_id;
  size_t user_id;
  size_t quantity;
  size_t price;
  ESide side;

  nlohmann::json as_json() const {
    return {{"order_id", order_id},
            {"user_id", user_id},
            {"quantity", quantity},
            {"price", price},
            {"side", side}};
  }
};

class OrderBook {
  friend class Core;

public:
  size_t addOrder(ESide side,
                  std::function<void(size_t, size_t, size_t)> trade_handler,
                  size_t user_id, size_t quantity, size_t price);

  const std::vector<Order> get_orders_by_user(size_t user_id) const;

  const Order &get_order_by_id(size_t order_id) const;

private:
  void processOrders(std::function<void(size_t, size_t, size_t)> trade_handler);

  struct CompareBuy {
    bool operator()(const Order &lhs, const Order &rhs) {
      return lhs.price < rhs.price;
    }
  };

  struct CompareSell {
    bool operator()(const Order &lhs, const Order &rhs) {
      return lhs.price > rhs.price;
    }
  };

  size_t next_order_id = 1;
  std::unordered_map<size_t, Order> orders_;
  std::priority_queue<Order, std::vector<Order>, CompareBuy> buy_orders_;
  std::priority_queue<Order, std::vector<Order>, CompareSell> sell_orders_;
};
