#include "order.hpp"
#include <chrono>

Order::Order(uint64_t order_id, Side side, double price, int quantity,
             int64_t timestamp_ns, OrderType type)
    : order_id{order_id}
    , side{side}
    , type{type}
    , price{price}
    , quantity{quantity}
    , timestamp_ns{timestamp_ns != 0
                   ? timestamp_ns
                   : std::chrono::duration_cast<std::chrono::nanoseconds>(
                         std::chrono::steady_clock::now().time_since_epoch()
                     ).count()}
{}
