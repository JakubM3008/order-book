/**
 * order_book.hpp — The two-sided sorted book
 *
 * KEY DATA STRUCTURES:
 *
 * std::map<double, PriceLevel, std::greater<double>>  bids_
 *   Java: new TreeMap<Double, PriceLevel>(Comparator.reverseOrder())
 *   - Keeps keys (prices) sorted. std::greater = descending. Highest bid first.
 *   - O(log n) insert, lookup, delete.
 *   - Backed by a red-black tree (same as Java TreeMap).
 *
 * std::map<double, PriceLevel>  asks_
 *   Java: new TreeMap<Double, PriceLevel>()
 *   - Default ascending order. Lowest ask first.
 *
 * std::unordered_map<uint64_t, std::pair<double, Side>>  order_map_
 *   Java: HashMap<Long, Map.Entry<Double, Side>>
 *   - O(1) average lookup by order_id.
 *   - Backed by a hash table (same as Java HashMap).
 *   - Stores (price, side) so we can jump directly to the right level for cancels.
 *
 * NOTE ON DOUBLE AS MAP KEY:
 * Using floating-point as a map key is generally dangerous (precision issues).
 * Real systems use integer ticks (e.g., price in units of 1/10000 of a cent).
 * We'll address this in the optimisation phase. For now, double is fine because
 * we only compare prices we've stored ourselves — no arithmetic between them.
 */
#pragma once

#include <map>
#include <unordered_map>
#include <optional>
#include <functional>  // std::greater
#include <string>
#include <vector>
#include "price_level.hpp"

class OrderBook {
public:
    explicit OrderBook(std::string symbol);

    // ------------------------------------------------------------------
    // Commands (modify state)
    // ------------------------------------------------------------------

    void                 add_limit_order(Order order);
    std::optional<Order> cancel_order(uint64_t order_id);

    // ------------------------------------------------------------------
    // Queries (read-only — all const)
    // ------------------------------------------------------------------

    /**
     * Returns a raw pointer to the best bid/ask PriceLevel, or nullptr if empty.
     *
     * Why a raw pointer and not std::optional?
     * std::optional<PriceLevel> would COPY the PriceLevel — wasteful.
     * std::optional<PriceLevel&> isn't legal in C++ (can't have optional of reference).
     *
     * A raw pointer here means: "non-owning reference, may be null".
     * The OrderBook owns the PriceLevel. We're just lending a view.
     * This is one legitimate use of raw pointers in modern C++.
     *
     * Java equivalent: @Nullable PriceLevel bestBid()
     * Caller checks: if (book.best_bid() != nullptr) { ... }
     *
     * [[nodiscard]] — compiler warning if caller ignores the return value.
     * Java: no equivalent (though some annotation processors do this).
     */
    [[nodiscard]] const PriceLevel* best_bid() const;
    [[nodiscard]] const PriceLevel* best_ask() const;

    [[nodiscard]] std::optional<double> mid_price() const;
    [[nodiscard]] std::optional<double> spread()    const;

    /**
     * Top N levels on one side. Returns by value (a new vector).
     * Each element is a const pointer into the book — no copying of PriceLevels.
     *
     * Java: List<PriceLevel> depth(Side side, int nLevels)
     */
    [[nodiscard]] std::vector<const PriceLevel*> depth(Side side, int n_levels = 5) const;

    /**
     * Total quantity available within bps basis points of mid price.
     * Directly answers: "how much can I liquidate within 5bps of fair value?"
     */
    [[nodiscard]] int liquidity_within_bps(Side side, double bps) const;

    const std::string& symbol() const { return symbol_; }

private:
    std::string symbol_;

    std::map<double, PriceLevel, std::greater<double>> bids_;  // descending
    std::map<double, PriceLevel>                       asks_;  // ascending

    std::unordered_map<uint64_t, std::pair<double, Side>> order_map_;

    // Internal helpers — private means they're implementation details, not part of the interface
    // Java: private methods
    std::map<double, PriceLevel, std::greater<double>>& bid_levels() { return bids_; }
    std::map<double, PriceLevel>&                       ask_levels() { return asks_; }
};
