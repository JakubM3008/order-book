#pragma once

#include <cstdint>
#include <chrono>

enum class Side : char {
    BUY  = 'B',
    SELL = 'S'
};

struct Order {
    uint64_t order_id;
    Side     side;
    double   price;
    int      quantity;
    int64_t  timestamp_ns;

    explicit Order(uint64_t order_id, Side side, double price, int quantity,
                   int64_t timestamp_ns = 0);
};
