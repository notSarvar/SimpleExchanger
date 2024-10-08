#pragma once

#include <string>

static short port = 5555;

namespace Requests {
static std::string Registration = "Reg";
static std::string MakeOrder = "MOrder";
static std::string ViewOrders = "VOrders";
static std::string ViewBalance = "VBalance";
static std::string ViewInfo = "VInfo";
} // namespace Requests

enum class EFields {
  EFUserId,
  EFOrderId,
  EFSide,
  EFQuantity,
  EFPrice,
  EFReqType,
  EFMessage
};

enum class ESide { ESell, EBuy };
