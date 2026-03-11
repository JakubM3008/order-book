#pragma once

#include <map>
#include <unordered_map>
#include <optional>
#include <functional>
#include <string>
#include <vector>
#include "price_level.hpp"

class OrderBook {
public:
    explicit OrderBook(std::string symbol);

    void                 add_limit_order(Order order);
    std::optional<Order> cancel_order(uint64_t order_id);

    [[nodiscard]] const PriceLevel* best_bid() const;
    [[nodiscard]] const PriceLevel* best_ask() const;

    [[nodiscard]] std::optional<double> mid_price() const;
    [[nodiscard]] std::optional<double> spread()    const;

    // Top N price levels on the given side, best-first.
    [[nodiscard]] std::vector<const PriceLevel*> depth(Side side, int n_levels = 5) const;

    // Total resting quantity within bps basis points of mid.
    [[nodiscard]] int liquidity_within_bps(Side side, double bps) const;

    const std::string& symbol() const { return symbol_; }

private:
    std::string symbol_;

    // bids: descending by price; asks: ascending.
    std::map<double, PriceLevel, std::greater<double>> bids_;
    std::map<double, PriceLevel>                       asks_;

    // order_id -> (price, side) for O(1) cancel routing.
    std::unordered_map<uint64_t, std::pair<double, Side>> order_map_;
};
