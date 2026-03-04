/**
 * order.hpp — The atomic unit of the order book
 *
 * JAVA vs C++ — the most important difference to understand first:
 *
 * In Java, when you write:
 *     Order o = new Order(...);
 * 'o' is a REFERENCE. The object lives on the heap. The GC cleans it up.
 * You never think about where it lives or how long it lives.
 *
 * In C++, you have a choice:
 *     Order o{...};           // lives on the STACK — destroyed automatically when scope ends
 *     Order* o = new Order{}; // lives on the HEAP  — YOU must delete it (or use smart pointers)
 *
 * For a small, plain data container like Order, stack allocation is almost always right.
 * It's faster (no heap allocation), cache-friendly, and lifetime is automatic.
 * We will never use raw 'new' in this project — see smart_pointers note below.
 *
 * HEADER FILES — no Java equivalent:
 * Java compiles .java files directly. The compiler reads everything.
 * C++ separates DECLARATION (what exists) from DEFINITION (how it works).
 *   - .hpp (header): declares types, function signatures — like a contract
 *   - .cpp (source):  defines the actual implementation
 * Every .cpp that needs to use Order #includes this header.
 * #pragma once ensures it's only processed once per compilation unit.
 */
#pragma once

#include <cstdint>   // uint64_t, int64_t — explicit-width integers (Java: long is always 64-bit; C++ int size varies by platform)
#include <chrono>    // high-resolution clock

// ---------------------------------------------------------------------------
// Side
// ---------------------------------------------------------------------------

/**
 * Java:  public enum Side { BUY, SELL }
 * C++:   enum class Side { BUY, SELL };
 *
 * 'enum class' (scoped enum) is the modern C++ equivalent.
 * Key differences:
 *   - Java enums are full objects (can have fields, methods). C++ enum class is just a named integer.
 *   - Java:  Side.BUY     C++: Side::BUY   (:: is the scope resolution operator, like . for statics)
 *   - C++ enum class does NOT implicitly convert to int (unlike old-style C enums).
 *     This is good — it prevents accidental comparisons like (side == 1).
 *
 * ': char' sets the underlying storage type. Without it, C++ uses int (4 bytes).
 * A char saves 3 bytes per Order. Multiplied by millions of orders, this matters.
 */
enum class Side : char {
    BUY  = 'B',
    SELL = 'S'
};

// ---------------------------------------------------------------------------
// Order
// ---------------------------------------------------------------------------

/**
 * Java equivalent:
 *   public record Order(long orderId, Side side, double price, int quantity, long timestampNs) {}
 * or a class with private fields + getters.
 *
 * C++ 'struct' vs 'class':
 *   Mechanically identical — the ONLY difference is default visibility.
 *   struct: members are PUBLIC by default.
 *   class:  members are PRIVATE by default.
 *
 * Convention: use struct for plain data (no invariants to protect), class for objects with behaviour.
 * Order is pure data — struct is appropriate.
 *
 * NO INHERITANCE, NO VIRTUAL METHODS here. This is intentional.
 * Virtual dispatch (Java's default for all methods) has overhead.
 * For a struct used in millions of containers, we want zero overhead.
 */
struct Order {
    uint64_t order_id;      // Java: long (always 64-bit). uint64_t is explicitly unsigned 64-bit.
    Side     side;          // 1 byte (char underlying type)
    double   price;         // Java: double — same IEEE 754, same size. Note: real systems use integer ticks.
    int      quantity;      // Java: int — same (32-bit signed on both)
    int64_t  timestamp_ns;  // nanoseconds since epoch. Java: long. int64_t = signed 64-bit.

    /**
     * Constructor.
     * Java: public Order(long orderId, Side side, double price, int quantity) { ... }
     * C++:  Order(uint64_t order_id, Side side, double price, int quantity);
     *
     * 'explicit' prevents the compiler from using this constructor for implicit conversions.
     * Example without explicit: Order o = 42; would try to construct from a single int. Bad.
     * With explicit: that line is a compile error. Always use explicit on single-arg constructors.
     * Multi-arg constructors don't need it (can't accidentally trigger them), but it's good habit.
     */
    explicit Order(uint64_t order_id, Side side, double price, int quantity,
                   int64_t timestamp_ns = 0);
};
