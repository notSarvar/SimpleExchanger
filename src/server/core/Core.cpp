#include "Core.h"
#include <cstddef>

template <ESide side, typename... Args>
std::unordered_set<size_t> &Core::make_order(Args &&...args) {
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

std::vector<nlohmann::json> &Core::get_to_send(size_t user_id) {
  return to_send_[user_id];
}

void Core::view_orders(size_t user_id) {
  nlohmann::json response;
  for (const auto &order : order_book_.get_orders_by_user(user_id)) {
    response += order.as_json();
  }
  to_send_[user_id].push_back(response);
}

void Core::user_info(size_t user_id) {
  auto response = users_[user_id].as_json();
  for (size_t order_id : users_[user_id].get_orders()) {
    const Order &order = order_book_.get_order_by_id(order_id);
    if (order.quantity) {
      response["ActiveOrders"].push_back(order.as_json());
    } else {
      response["NonActiveOrders"].push_back(order.as_json());
    }
  }
  to_send_[user_id].push_back(response);
}

size_t Core::reg() {
  ++last_user_id_;
  users_[last_user_id_].set_user_id(last_user_id_);
  auto response = users_[last_user_id_].as_json();
  to_send_[last_user_id_].push_back(response);
  return last_user_id_;
}

size_t Core::get_last_user_id() const { return last_user_id_; }
