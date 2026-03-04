#include "order_book.hpp"
#include <stdexcept>
#include <cmath>

OrderBook::OrderBook(std::string symbol)
    : symbol_{std::move(symbol)}
{}

// ---------------------------------------------------------------------------
// add_limit_order
// ---------------------------------------------------------------------------

void OrderBook::add_limit_order(Order order) {
    if (order_map_.count(order.order_id)) {
        throw std::invalid_argument("Duplicate order_id");
    }

    // Capture BEFORE std::move. After a move, reading the object is undefined behaviour.
    // This is a real C++ footgun — Java has no move so you never hit this.
    const uint64_t id    = order.order_id;
    const double   price = order.price;
    const Side     side  = order.side;

    if (side == Side::BUY) {
        /**
         * try_emplace(key, constructor_args...)
         * Java: bids.computeIfAbsent(price, p -> new PriceLevel(p))
         *
         * Inserts PriceLevel(price) at key=price only if absent.
         * Constructs in-place — no temporary, no copy.
         */
        bids_.try_emplace(price, price);
        bids_.at(price).add_order(std::move(order));
    } else {
        asks_.try_emplace(price, price);
        asks_.at(price).add_order(std::move(order));
    }

    // Record (price, side) for O(1) cancel lookup
    order_map_.emplace(id, std::make_pair(price, side));
    // Java: orderMap.put(id, Map.entry(price, side));
}

// ---------------------------------------------------------------------------
// cancel_order
// ---------------------------------------------------------------------------

std::optional<Order> OrderBook::cancel_order(uint64_t order_id) {
    auto it = order_map_.find(order_id);
    if (it == order_map_.end()) {
        return std::nullopt;  // Java: return Optional.empty();
    }

    /**
     * Structured binding (C++17) — unpacks std::pair in one line.
     * Java: double price = it.getValue().getKey();
     *       Side   side  = it.getValue().getValue();
     */
    auto [price, side] = it->second;
    order_map_.erase(it);  // erase by iterator — O(1)

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

// ---------------------------------------------------------------------------
// Queries
// ---------------------------------------------------------------------------

const PriceLevel* OrderBook::best_bid() const {
    if (bids_.empty()) return nullptr;
    /**
     * bids_ sorted descending (std::greater), so begin() = highest price = best bid.
     * ->second = the PriceLevel value in the key-value pair.
     * & takes its address — returns a non-owning pointer (we don't transfer ownership).
     * Java: bids.firstEntry().getValue()
     */
    return &bids_.begin()->second;
}

const PriceLevel* OrderBook::best_ask() const {
    if (asks_.empty()) return nullptr;
    return &asks_.begin()->second;  // ascending sort — begin() = lowest ask
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
    result.reserve(n_levels);  // Java: new ArrayList<>(nLevels) — hint the capacity

    /**
     * Range-based for with structured binding over the map.
     * Java: for (var entry : asks.entrySet()) { var level = entry.getValue(); ... }
     * C++:  for (const auto& [price, level] : asks_)
     *
     * 'const auto&' — reference, no copy. In Java this is automatic for objects.
     * In C++ you must ask for it. Omitting & here would copy every PriceLevel — very wrong.
     */
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

    const double mid       = *mid_opt;  // dereference optional — like *pointer, safe after null check
    const double tolerance = mid * (bps / 10000.0);
    int total = 0;

    if (side == Side::SELL) {
        for (const auto& [price, level] : asks_) {
            if (price - mid <= tolerance) total += level.total_quantity();
            else break;  // map is sorted — everything past here is also out of range
        }
    } else {
        for (const auto& [price, level] : bids_) {
            if (mid - price <= tolerance) total += level.total_quantity();
            else break;
        }
    }
    return total;
}
