#include "price_level.hpp"

PriceLevel::PriceLevel(double price)
    : price_{price}
{}

void PriceLevel::add_order(Order order) {
    total_quantity_ += order.quantity;
    orders_.push_back(std::move(order));
}

std::optional<Order> PriceLevel::remove_order(uint64_t order_id) {
    for (auto it = orders_.begin(); it != orders_.end(); ++it) {
        if (it->order_id == order_id) {
            Order removed = *it;
            total_quantity_ -= removed.quantity;
            orders_.erase(it);
            return removed;
        }
    }
    return std::nullopt;
}
