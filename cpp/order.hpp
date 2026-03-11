#pragma once

#include <cstdint>
#include <chrono>

enum class Side : char {
    BUY  = 'B',
    SELL = 'S'
};

enum class OrderType : char {
    LIMIT  = 'L',  // rest at price until filled or cancelled
    MARKET = 'M',  // fill at any price, sweep the book
    IOC    = 'I',  // fill what crosses immediately, cancel the rest
    FOK    = 'F'   // fill entire quantity immediately or cancel entirely
};
struct Order {
    uint64_t  order_id;
    Side      side;
    OrderType type;
    double    price;     // ignored for MARKET orders
    int       quantity;
    int64_t   timestamp_ns;

    explicit Order(uint64_t order_id, Side side, double price, int quantity,
                   int64_t timestamp_ns = 0,
                   OrderType type = OrderType::LIMIT);
};
