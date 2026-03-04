/**
 * price_level.hpp — All orders resting at one price
 *
 * WHY std::deque? (The design question you raised)
 *
 * We need a container that supports:
 *   1. Append to back:          new orders join the back of the queue
 *   2. Remove from front:       when an order is filled, it leaves from the front
 *   3. Random removal by index: cancel can hit any order in the queue, not just front/back
 *
 * Let's compare the candidates (Java equivalents shown):
 *
 *   std::vector  (Java: ArrayList)
 *     + O(1) push_back, O(1) random access, excellent cache performance (contiguous memory)
 *     - O(n) pop_front — has to shift every element left. Painful on a busy level.
 *     - O(n) erase from middle (same shift problem)
 *     ✗ Bad for FIFO queue with front removal.
 *
 *   std::list  (Java: LinkedList)
 *     + O(1) insert/remove ANYWHERE if you have an iterator
 *     - Pointer-chasing: each element has prev/next pointers, memory is scattered
 *     - Terrible cache performance — modern CPUs hate non-contiguous access
 *     - No random access — you must iterate to find position i
 *     ✗ Cache miss cost outweighs the O(1) remove benefit in practice.
 *
 *   std::queue  (Java: ArrayDeque used as a Queue)
 *     + O(1) push (back) and pop (front)
 *     - It's an ADAPTOR — it wraps deque but hides the underlying container
 *     - No iterator access, no random removal — we can't reach inside to cancel
 *     ✗ Doesn't expose what we need for cancels.
 *
 *   std::deque  (Java: ArrayDeque)
 *     + O(1) push_back and pop_front — perfect FIFO
 *     + Supports iterators — we can scan and erase for cancels
 *     + Better cache than list (stored in fixed-size chunks, not scattered)
 *     - Not fully contiguous (chunked), so slightly worse cache than vector
 *     - O(n) erase from middle (same as vector, but front removal is O(1))
 *     ✓ Best fit: FIFO with occasional mid-sequence cancel, iterable.
 *
 * The O(n) cancel is acceptable here because:
 *   a) Most price levels are shallow (few orders deep in practice)
 *   b) The order_map in OrderBook gets us to the RIGHT LEVEL in O(1) —
 *      we only scan within one level, not the whole book.
 *
 * In a production HFT system, you'd replace this with an intrusive linked list
 * (O(1) removal anywhere) with a separate price-indexed array. We'll revisit that
 * in the optimisation phase.
 */
#pragma once

#include <deque>
#include <optional>   // C++17 — Java: Optional<T>
#include "order.hpp"

class PriceLevel {
public:
    /**
     * 'explicit' on a single-argument constructor prevents implicit conversion.
     * Java: public PriceLevel(double price) { this.price = price; }
     */
    explicit PriceLevel(double price);

    // ------------------------------------------------------------------
    // Mutators
    // ------------------------------------------------------------------

    /**
     * Add an order to the back of the queue.
     *
     * PASS BY VALUE — 'Order order' not 'const Order& order'.
     * Java always passes objects by reference. C++ gives you a choice.
     *
     * Why value here? We're going to STORE the order in the deque anyway,
     * so a copy must happen at some point. Taking by value lets the compiler
     * apply "move semantics" — if the caller passes a temporary Order,
     * the compiler moves it (zero-copy) into the parameter, then moves it
     * again into the deque. No redundant copies.
     *
     * If we took const Order&, the compiler couldn't move — it would copy.
     * Rule of thumb: take by value when you're going to store/move the argument.
     */
    void add_order(Order order);

    /**
     * Remove a specific order by ID. Returns the removed order, or nothing.
     *
     * std::optional<Order> — Java: Optional<Order>
     * Represents a value that may or may not be present.
     * Much safer than returning null (no NullPointerException possible).
     * Caller must explicitly check: if (result.has_value()) { ... }
     * or use: result.value_or(default)
     *
     * std::nullopt is the C++ equivalent of Optional.empty()
     */
    std::optional<Order> remove_order(uint64_t order_id);

    // ------------------------------------------------------------------
    // Accessors — all marked 'const'
    // ------------------------------------------------------------------
    /**
     * 'const' at the end of a method signature = "this method does not modify the object".
     * Java has no direct equivalent — final only applies to fields, not methods.
     *
     * This matters because:
     *   1. The compiler enforces it — you can't accidentally modify state in a const method.
     *   2. You can call const methods on const references (read-only views of an object).
     *      const PriceLevel& level = ...; level.price(); // OK
     *      level.add_order(...);                          // compile error — add_order is not const
     *
     * Think of it as: Java's read-only interface, but enforced at compile time.
     */
    double price()          const { return price_; }
    int    total_quantity() const { return total_quantity_; }
    bool   empty()          const { return orders_.empty(); }

    /**
     * Return a const reference to the internal deque — read-only view.
     *
     * 'const std::deque<Order>&' — Java: Collections.unmodifiableCollection(orders)
     * The & means reference (no copy). const means the caller can't modify it.
     * No heap allocation, no copy — just a view into the existing data.
     */
    const std::deque<Order>& orders() const { return orders_; }

private:
    /**
     * Private fields — trailing underscore is a common C++ convention for member variables.
     * Java convention: this.price. C++ doesn't need 'this->' in normal usage.
     *
     * In-class initialisation (= 0) sets the default.
     * Java: private int totalQuantity = 0;  — same idea.
     */
    double            price_;
    std::deque<Order> orders_;
    int               total_quantity_ = 0;
};
