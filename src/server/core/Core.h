#pragma once

#include "src/server/include/Common.h"
#include "src/server/core/orders/Orders.cpp"
#include "src/server/include/Singleton.hpp"
#include "src/server/core/user/User.cpp"
#include "import/json/json.hpp"

#include <unordered_map>
#include <unordered_set>

class Core : public Singleton<Core> {
public:
  template <ESide side, typename... Args>
  std::unordered_set<size_t> &make_order(Args &&...args);

  void view_orders(size_t user_id);

  void user_info(size_t user_id);

  size_t get_last_user_id() const;

  std::vector<nlohmann::json> &get_to_send(size_t user_id);

  size_t reg();

private:
  OrderBook order_book_;
  std::unordered_map<size_t, UserInfo> users_;
  std::unordered_map<size_t, std::vector<nlohmann::json>> to_send_;
  size_t last_user_id_ = 0;
  std::unordered_set<size_t> to_notify_;
};
