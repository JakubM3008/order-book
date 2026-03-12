#include "order_book.hpp"
#include <stdexcept>
#include <cmath>

OrderBook::OrderBook(std::string symbol)
    : symbol_{std::move(symbol)}
{}

void OrderBook::add_limit_order(Order order) {
    if (order_map_.count(order.order_id)) {
        throw std::invalid_argument("Duplicate order_id");
    }

    // Capture fields before std::move invalidates the object.
    const uint64_t id    = order.order_id;
    const double   price = order.price;
    const Side     side  = order.side;

    if (side == Side::BUY) {
        auto [it, _] = bids_.try_emplace(price, price);
        it->second.add_order(std::move(order));
    } else {
        auto [it, _] = asks_.try_emplace(price, price);
        it->second.add_order(std::move(order));
    }

    order_map_.emplace(id, std::make_pair(price, side));
}

std::optional<Order> OrderBook::cancel_order(uint64_t order_id) {
    auto it = order_map_.find(order_id);
    if (it == order_map_.end()) return std::nullopt;

    auto [price, side] = it->second;
    order_map_.erase(it);

    if (side == Side::BUY) {
        auto level_it = bids_.find(price);
        if (level_it == bids_.end()) return std::nullopt;
        auto cancelled = level_it->second.remove_order(order_id);
        if (level_it->second.empty()) bids_.erase(level_it);
        return cancelled;
    } else {
        auto level_it = asks_.find(price);
        if (level_it == asks_.end()) return std::nullopt;
        auto cancelled = level_it->second.remove_order(order_id);
        if (level_it->second.empty()) asks_.erase(level_it);
        return cancelled;
    }
}

const PriceLevel* OrderBook::best_bid() const {
    if (bids_.empty()) return nullptr;
    return &bids_.begin()->second;
}

const PriceLevel* OrderBook::best_ask() const {
    if (asks_.empty()) return nullptr;
    return &asks_.begin()->second;
}

std::optional<double> OrderBook::mid_price() const {
    const auto* bid = best_bid();
    const auto* ask = best_ask();
    if (!bid || !ask) return std::nullopt;
    return (bid->price() + ask->price()) / 2.0;
}

std::optional<double> OrderBook::spread() const {
    const auto* bid = best_bid();
    const auto* ask = best_ask();
    if (!bid || !ask) return std::nullopt;
    return ask->price() - bid->price();
}

std::vector<const PriceLevel*> OrderBook::depth(Side side, int n_levels) const {
    std::vector<const PriceLevel*> result;
    result.reserve(n_levels);

    if (side == Side::BUY) {
        for (const auto& [price, level] : bids_) {
            if (static_cast<int>(result.size()) >= n_levels) break;
            result.push_back(&level);
        }
    } else {
        for (const auto& [price, level] : asks_) {
            if (static_cast<int>(result.size()) >= n_levels) break;
            result.push_back(&level);
        }
    }
    return result;
}

int OrderBook::liquidity_within_bps(Side side, double bps) const {
    auto mid_opt = mid_price();
    if (!mid_opt) return 0;

    const double mid       = *mid_opt;
    const double tolerance = mid * (bps / 10000.0);
    int total = 0;

    if (side == Side::SELL) {
        for (const auto& [price, level] : asks_) {
            if (price - mid <= tolerance) total += level.total_quantity();
            else break;
        }
    } else {
        for (const auto& [price, level] : bids_) {
            if (mid - price <= tolerance) total += level.total_quantity();
            else break;
        }
    }
    return total;
}
