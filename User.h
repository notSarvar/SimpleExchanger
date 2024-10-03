#pragma once

#include "json.hpp"

#include <vector>

class UserInfo {
public:
  nlohmann::json as_json() const;

  std::vector<size_t> &get_orders();

  const std::vector<size_t> &get_orders() const;

  void add_usd(int val);

  void add_rub(int val);

  void set_user_id(size_t id);

  void add_order(size_t order_id);

private:
  std::vector<size_t> orders_;
  size_t user_id_;
  int rub_amount_;
  int usd_amount_;
};
