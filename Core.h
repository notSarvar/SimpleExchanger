#pragma once

#include "Common.hpp"
#include "Orders.cpp"
#include "Singleton.hpp"
#include "User.h"
#include "json.hpp"

#include <unordered_map>
#include <unordered_set>

class Core : public Singleton<Core> {
public:
  template <ESide side, typename... Args>
  std::unordered_set<size_t> make_order(Args &&...args) {
    auto trade_handler = [this](size_t our_order_id, size_t other_order_id,
                                size_t quantity) {
      const Order &our_order = order_book_.get_order_by_id(our_order_id);
      const Order &other_order = order_book_.get_order_by_id(other_order_id);
      to_send_[other_order.user_id].push_back(other_order.as_json());
      if (our_order.user_id != other_order.user_id) {
        to_notify_.insert(other_order.user_id);
        if constexpr (side == ESide::ESell) {
          users_[other_order.user_id].add_usd(quantity);
          users_[other_order.user_id].add_rub(-quantity * other_order.price);
          users_[our_order.user_id].add_usd(-quantity);
          users_[our_order.user_id].add_rub(quantity * other_order.price);
        } else {
          users_[other_order.user_id].add_usd(-quantity);
          users_[other_order.user_id].add_rub(quantity * other_order.price);
          users_[our_order.user_id].add_usd(quantity);
          users_[our_order.user_id].add_rub(-quantity * other_order.price);
        }
      }
    };
    size_t order_id =
        order_book_.addOrder(side, trade_handler, std::forward<Args>(args)...);
    const Order &order = order_book_.get_order_by_id(order_id);
    users_[order.user_id].add_order(order_id);
    return to_notify_;
  }

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
