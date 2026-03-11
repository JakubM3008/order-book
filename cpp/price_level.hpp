#pragma once

#include <deque>
#include <optional>
#include "order.hpp"

// All resting orders at a single price point, maintained in arrival order (FIFO).
class PriceLevel {
public:
    explicit PriceLevel(double price);

    void                 add_order(Order order);
    std::optional<Order> remove_order(uint64_t order_id);

    double price()          const { return price_; }
    int    total_quantity() const { return total_quantity_; }
    bool   empty()          const { return orders_.empty(); }

    const std::deque<Order>& orders() const { return orders_; }

private:
    double            price_;
    std::deque<Order> orders_;
    int               total_quantity_ = 0;
};
