#include "price_level.hpp"

// Constructor — member initialiser list, body is empty
PriceLevel::PriceLevel(double price)
    : price_{price}
{}

void PriceLevel::add_order(Order order) {
    total_quantity_ += order.quantity;
    /**
     * std::move — this is "move semantics", one of the key C++ concepts with no Java equivalent.
     *
     * Java: every object assignment is a reference copy. No data is duplicated.
     * C++:  'order' here is a local variable (a value). Copying it into the deque
     *        would duplicate all its bytes. For a small struct like Order that's fine,
     *        but for large objects it's wasteful.
     *
     * std::move(order) says: "I'm done with 'order' — transfer its guts into the deque,
     * don't copy them." After the move, 'order' is in a valid-but-unspecified state
     * (we don't use it again, so we don't care).
     *
     * For Order (just primitives), move == copy. But writing std::move is correct idiom
     * and matters when we later have types with heap-allocated members (strings, vectors).
     */
    orders_.push_back(std::move(order));
}

std::optional<Order> PriceLevel::remove_order(uint64_t order_id) {
    /**
     * Iterator-based loop — Java equivalent:
     *   Iterator<Order> it = orders.iterator();
     *   while (it.hasNext()) {
     *     Order o = it.next();
     *     if (o.orderId == orderId) { it.remove(); ... }
     *   }
     *
     * C++ iterators are pointer-like objects. it->order_id dereferences the iterator
     * and accesses the field — same as (*it).order_id.
     *
     * orders_.begin() = iterator to first element
     * orders_.end()   = iterator to ONE PAST the last element (sentinel, never dereference)
     * ++it             = advance to next element
     */
    for (auto it = orders_.begin(); it != orders_.end(); ++it) {
        if (it->order_id == order_id) {
            // Found it — capture before erasing
            Order removed = *it;   // copy the order out before we destroy it
            total_quantity_ -= removed.quantity;
            orders_.erase(it);     // O(n) — shifts elements. Acceptable at level depth.
            return removed;        // implicitly wraps in std::optional
        }
    }
    return std::nullopt;           // Java: return Optional.empty();
}
