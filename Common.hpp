#ifndef CLIENSERVERECN_COMMON_HPP
#define CLIENSERVERECN_COMMON_HPP

#include <string>

static short port = 5555;

namespace Requests {
static std::string Registration = "Reg";
static std::string Hello = "Hello";
static std::string MakeOrder = "MOrder";
static std::string ViewOrders = "VOreders";
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

#endif // CLIENSERVERECN_COMMON_HPP
