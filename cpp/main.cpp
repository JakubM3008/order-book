#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <atomic>

#include "order_book.hpp"

static std::atomic<uint64_t> next_order_id{1};

uint64_t next_id() {
    return next_order_id.fetch_add(1);
}

void print_separator(int width = 60) {
    std::cout << std::string(width, '-') << '\n';
}

void display_book(const OrderBook& book, int n_levels = 5) {
    auto mid_opt    = book.mid_price();
    auto spread_opt = book.spread();

    print_separator();
    std::cout << "  Order Book: " << book.symbol() << '\n';

    if (mid_opt && spread_opt) {
        double mid        = *mid_opt;
        double spread     = *spread_opt;
        double spread_bps = (spread / mid) * 10000.0;

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

    for (int i = static_cast<int>(asks.size()) - 1; i >= 0; --i) {
        const auto* level = asks[i];
        std::cout << std::setw(20) << ""
                  << std::setw(12) << std::fixed << std::setprecision(4) << level->price()
                  << std::setw(20) << level->total_quantity() << '\n';
    }

    std::cout << std::setw(44) << "---- SPREAD ----" << '\n';

    for (const auto* level : bids) {
        std::cout << std::setw(20) << level->total_quantity()
                  << std::setw(12) << std::fixed << std::setprecision(4) << level->price()
                  << '\n';
    }
    print_separator();
    std::cout << '\n';
}

int main() {
    OrderBook book{"UST_10Y"};

    // Resting limit orders — these live in the book until filled or cancelled.
    book.add_limit_order(Order{next_id(), Side::SELL, 99.0625, 50'000'000});
    book.add_limit_order(Order{next_id(), Side::SELL, 99.0625, 25'000'000});
    book.add_limit_order(Order{next_id(), Side::SELL, 99.0312, 75'000'000});
    book.add_limit_order(Order{next_id(), Side::SELL, 99.0937, 30'000'000});
    book.add_limit_order(Order{next_id(), Side::SELL, 99.1250, 20'000'000});

    book.add_limit_order(Order{next_id(), Side::BUY,  99.0000, 60'000'000});
    book.add_limit_order(Order{next_id(), Side::BUY,  99.0000, 40'000'000});
    book.add_limit_order(Order{next_id(), Side::BUY,  98.9687, 80'000'000});
    book.add_limit_order(Order{next_id(), Side::BUY,  98.9375, 50'000'000});
    book.add_limit_order(Order{next_id(), Side::BUY,  98.9062, 35'000'000});

    display_book(book);

    // Examples of aggressive order types — handled by the matching engine, not stored here.
    // Shown to demonstrate how they would be constructed and dispatched.
    Order market_buy {next_id(), Side::BUY,  0.0,     200'000'000, 0, OrderType::MARKET};
    Order ioc_sell   {next_id(), Side::SELL, 99.0000, 100'000'000, 0, OrderType::IOC};
    Order fok_buy    {next_id(), Side::BUY,  99.0312,  75'000'000, 0, OrderType::FOK};

    std::cout << "Aggressive orders constructed (matching engine handles these):\n";
    std::cout << "  MARKET BUY  200mm — sweep asks at any price\n";
    std::cout << "  IOC    SELL 100mm @ 99.0000 — fill what crosses, cancel rest\n";
    std::cout << "  FOK    BUY   75mm @ 99.0312 — fill all or cancel entirely\n\n";

    std::cout << "Liquidity within 5bps (ask side): "
              << book.liquidity_within_bps(Side::SELL, 5.0) << '\n';
    std::cout << "Liquidity within 5bps (bid side): "
              << book.liquidity_within_bps(Side::BUY, 5.0) << '\n';

    std::cout << "\nCancelling order 1...\n";
    auto cancelled = book.cancel_order(1);
    if (cancelled) {
        std::cout << "Cancelled order_id=" << cancelled->order_id
                  << " qty=" << cancelled->quantity << '\n';
    }

    display_book(book);

    return 0;
}
