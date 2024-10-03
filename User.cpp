#include "User.h"

nlohmann::json UserInfo::as_json() const {
  nlohmann::json res;
  res["UserId"] = user_id_;
  res["RUB"] = rub_amount_;
  res["USD"] = usd_amount_;
  return res;
}

std::vector<size_t> &UserInfo::get_orders() { return orders_; }

const std::vector<size_t> &UserInfo::get_orders() const { return orders_; }

void UserInfo::add_usd(int val) { usd_amount_ += val; }

void UserInfo::add_rub(int val) { rub_amount_ += val; }

void UserInfo::set_user_id(size_t id) { user_id_ = id; }

void UserInfo::add_order(size_t order_id) { orders_.push_back(order_id); };
