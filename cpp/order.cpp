/**
 * order.cpp — Implementation of Order
 *
 * This is the .cpp counterpart to order.hpp.
 * The header declared WHAT Order is. This file defines HOW it works.
 *
 * In Java, declaration and definition live in the same .java file.
 * In C++, the compiler needs to see the declaration (header) before it can
 * compile any code that uses the type — hence #include.
 */
#include "order.hpp"

#include <chrono>

/**
 * Constructor definition.
 *
 * 'Order::' — the scope resolution operator tells the compiler this is the
 * constructor that belongs to the Order struct, not some free function.
 * Java equivalent: this is just the constructor body inside the class file.
 *
 * MEMBER INITIALISER LIST — the ': order_id(order_id), side(side), ...' part.
 * Java: you assign fields in the constructor body: this.orderId = orderId;
 * C++:  prefer the initialiser list — it constructs members directly rather
 *       than default-constructing them first and then assigning.
 *       For primitives it makes no difference. For complex types it matters a lot.
 *
 * If timestamp_ns is 0 (default), we fill it from the system clock.
 */
Order::Order(uint64_t order_id, Side side, double price, int quantity,
             int64_t timestamp_ns)
    : order_id{order_id}
    , side{side}
    , price{price}
    , quantity{quantity}
    , timestamp_ns{timestamp_ns != 0
                   ? timestamp_ns
                   : std::chrono::duration_cast<std::chrono::nanoseconds>(
                         std::chrono::steady_clock::now().time_since_epoch()
                     ).count()}
{}
// The {} at the end is the constructor body — empty because all work happened
// in the initialiser list. This is idiomatic C++.
