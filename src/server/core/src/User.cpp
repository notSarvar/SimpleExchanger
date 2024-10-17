#include <json/json.hpp>

#include <vector>

class UserInfo {
public:
  nlohmann::json as_json() const {
    nlohmann::json res;
    res["UserId"] = user_id_;
    res["RUB"] = rub_amount_;
    res["USD"] = usd_amount_;
    return res;
  }

  std::vector<size_t> &get_orders() { return orders_; }

  const std::vector<size_t> &get_orders() const { return orders_; }

  void add_usd(int val) { usd_amount_ += val; }

  void add_rub(int val) { rub_amount_ += val; }

  void set_user_id(size_t id) { user_id_ = id; }

  void add_order(size_t order_id) { orders_.push_back(order_id); };

private:
  std::vector<size_t> orders_;
  size_t user_id_;
  int rub_amount_;
  int usd_amount_;
};
