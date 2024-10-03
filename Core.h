#pragma once

#include "Common.hpp"
#include "Singleton.hpp"
#include "User.h"
#include "json.hpp"

#include <unordered_map>
#include <unordered_set>

class Core : public Singleton<Core> {
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
  public:
    size_t addOrder(ESide side,
                    std::function<void(size_t, size_t, size_t)> trade_handler,
                    size_t user_id, size_t quantity, size_t price) {
      size_t order_id = next_order_id++;
      Order order = {order_id, user_id, quantity, price, side};
      orders_[order_id] = order;

      if (side == ESide::EBuy) {
        buy_orders_.push(order);
      } else {
        sell_orders_.push(order);
      }

      processOrders(trade_handler);
      return order_id;
    }

    const std::vector<Order> get_orders_by_user(size_t user_id) const {
      std::vector<Order> user_orders;
      for (const auto &[order_id, order] : orders_) {
        if (order.user_id == user_id) {
          user_orders.push_back(order);
        }
      }
      return user_orders;
    }

    const Order &get_order_by_id(size_t order_id) const {
      return orders_.at(order_id);
    }

  private:
    void
    processOrders(std::function<void(size_t, size_t, size_t)> trade_handler) {
      while (!buy_orders_.empty() && !sell_orders_.empty()) {
        Order buy_order = buy_orders_.top();
        Order sell_order = sell_orders_.top();

        if (buy_order.price >= sell_order.price) {
          size_t trade_quantity =
              std::min(buy_order.quantity, sell_order.quantity);
          trade_handler(buy_order.order_id, sell_order.order_id,
                        trade_quantity);

          orders_[buy_order.order_id].quantity -= trade_quantity;
          orders_[sell_order.order_id].quantity -= trade_quantity;

          if (orders_[buy_order.order_id].quantity == 0) {
            buy_orders_.pop();
          }
          if (orders_[sell_order.order_id].quantity == 0) {
            sell_orders_.pop();
          }
        } else {
          break;
        }
      }
    }

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

public:
  template <ESide side, typename... Args>
  std::unordered_set<size_t> &make_order(Args &&...args) {
    to_notify_.clear();
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
    to_send_[order.user_id].push_back(order.as_json());
    return to_notify_;
  }

  void view_orders(size_t user_id);

  void user_info(size_t user_id);

  std::vector<nlohmann::json> &get_to_send(size_t user_id);

  size_t reg();

private:
  OrderBook order_book_;
  std::unordered_map<size_t, UserInfo> users_;
  std::unordered_map<size_t, std::vector<nlohmann::json>> to_send_;
  size_t last_user_id_ = 0;
  std::unordered_set<size_t> to_notify_;
};
