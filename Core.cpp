#include "core.h"
#include <cstddef>

// std::vector<nlohmann::json> &Core::get_to_send(size_t user_id) {
//   return to_send_[user_id];
// }

void Core::view_orders(size_t user_id) {
  nlohmann::json response;
  for (const auto &order : order_book_.get_orders_by_user(user_id)) {
    response += order.as_json();
  }
  // to_send_[user_id].push_back(response);
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
  // to_send_[user_id].push_back(response);
}

size_t Core::reg() {
  ++last_user_id_;
  users_[last_user_id_].set_user_id(last_user_id_);
  auto response = users_[last_user_id_].as_json();
  // to_send_[last_user_id_].push_back(response);
  return last_user_id_;
}

size_t Core::get_last_user_id() const { return last_user_id_; }

std::string Core::get_info(size_t user_id) {
  return users_[user_id].as_json().dump();
}