/**
 * main.cpp — Demo: same scenario as the Python notebook
 *
 * In Java: public static void main(String[] args)
 * In C++:  int main()  — returns int (exit code). 0 = success.
 *          The OS/runtime calls main. No class needed — free functions exist in C++.
 */
#include <iostream>   // std::cout — Java: System.out
#include <iomanip>    // std::setw, std::fixed, std::setprecision — Java: String.format
#include <string>
#include <vector>
#include <atomic>     // std::atomic for thread-safe ID generation

#include "order_book.hpp"

// ---------------------------------------------------------------------------
// ID generator
// ---------------------------------------------------------------------------

/**
 * std::atomic<uint64_t> — thread-safe counter, no locks needed for increment.
 * Java: java.util.concurrent.atomic.AtomicLong
 *
 * 'static' here means "file-scoped" — not visible outside this .cpp.
 * Different from Java's static (which means class-level).
 */
static std::atomic<uint64_t> next_order_id{1};

uint64_t next_id() {
    return next_order_id.fetch_add(1);  // Java: nextOrderId.getAndIncrement()
}

// ---------------------------------------------------------------------------
// Display helpers
// ---------------------------------------------------------------------------

void print_separator(int width = 60) {
    std::cout << std::string(width, '-') << '\n';
}

void display_book(const OrderBook& book, int n_levels = 5) {
    /**
     * Passing by const reference — Java passes objects by reference automatically.
     * C++ requires explicit '&'. Without it, display_book would COPY the entire OrderBook.
     * 'const' means we promise not to modify it inside this function.
     */
    auto mid_opt    = book.mid_price();
    auto spread_opt = book.spread();

    print_separator();
    std::cout << "  Order Book: " << book.symbol() << '\n';

    if (mid_opt && spread_opt) {
        double mid        = *mid_opt;
        double spread     = *spread_opt;
        double spread_bps = (spread / mid) * 10000.0;

        // std::fixed + std::setprecision — Java: String.format("%.6f", mid)
        std::cout << std::fixed << std::setprecision(6)
                  << "  Mid: " << mid
                  << "   Spread: " << std::setprecision(4) << spread
                  << " (" << std::setprecision(2) << spread_bps << " bps)\n";
    }

    print_separator();
    std::cout << std::setw(20) << "BID QTY"
              << std::setw(12) << "PRICE"
              << std::setw(20) << "ASK QTY" << '\n';
    print_separator();

    auto asks = book.depth(Side::SELL, n_levels);
    auto bids = book.depth(Side::BUY,  n_levels);

    // Print asks in reverse (worst ask at top, best ask nearest spread)
    for (int i = static_cast<int>(asks.size()) - 1; i >= 0; --i) {
        const auto* level = asks[i];
        std::cout << std::setw(20) << ""
                  << std::setw(12) << std::fixed << std::setprecision(4) << level->price()
                  << std::setw(20) << level->total_quantity() << '\n';
    }

    std::cout << std::setw(44) << "---- SPREAD ----" << '\n';

    // Print bids (best bid nearest spread, worst bid at bottom)
    for (const auto* level : bids) {
        std::cout << std::setw(20) << level->total_quantity()
                  << std::setw(12) << std::fixed << std::setprecision(4) << level->price()
                  << '\n';
    }
    print_separator();
    std::cout << '\n';
}

// ---------------------------------------------------------------------------
// main
// ---------------------------------------------------------------------------

int main() {
    OrderBook book{"UST_10Y"};

    // ---- Ask side ----
    book.add_limit_order(Order{next_id(), Side::SELL, 99.0625, 50'000'000});
    book.add_limit_order(Order{next_id(), Side::SELL, 99.0625, 25'000'000});  // same level
    book.add_limit_order(Order{next_id(), Side::SELL, 99.0312, 75'000'000});  // better ask
    book.add_limit_order(Order{next_id(), Side::SELL, 99.0937, 30'000'000});
    book.add_limit_order(Order{next_id(), Side::SELL, 99.1250, 20'000'000});

    // ---- Bid side ----
    book.add_limit_order(Order{next_id(), Side::BUY,  99.0000, 60'000'000});
    book.add_limit_order(Order{next_id(), Side::BUY,  99.0000, 40'000'000});  // same level
    book.add_limit_order(Order{next_id(), Side::BUY,  98.9687, 80'000'000});
    book.add_limit_order(Order{next_id(), Side::BUY,  98.9375, 50'000'000});
    book.add_limit_order(Order{next_id(), Side::BUY,  98.9062, 35'000'000});
    // Note: 50'000'000 — digit separators (C++14). Java: 50_000_000. Same idea.

    display_book(book);

    // --- Liquidity queries ---
    std::cout << "Liquidity within 5bps (ask side, available to buy): "
              << book.liquidity_within_bps(Side::SELL, 5.0) << '\n';
    std::cout << "Liquidity within 5bps (bid side, available to sell): "
              << book.liquidity_within_bps(Side::BUY, 5.0) << '\n';

    // --- Cancel ---
    std::cout << "\nCancelling order 1 (50mm ask at 99.0625)...\n";
    auto cancelled = book.cancel_order(1);

    /**
     * Checking std::optional — Java: if (cancelled.isPresent()) { ... }
     * C++ options:
     *   if (cancelled.has_value()) { ... }
     *   if (cancelled) { ... }           // optional converts to bool
     *   cancelled.value()                // throws std::bad_optional_access if empty
     *   *cancelled                       // dereference like pointer — UB if empty, use carefully
     */
    if (cancelled) {
        std::cout << "Cancelled order_id=" << cancelled->order_id
                  << " qty=" << cancelled->quantity << '\n';
    }

    display_book(book);

    return 0;  // Java: implicit. C++: explicit (though compiler assumes 0 if omitted in main).
}
