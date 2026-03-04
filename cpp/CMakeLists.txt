cmake_minimum_required(VERSION 3.16)
project(order_book CXX)

# Require C++17 — we use structured bindings, std::optional, if-with-initialiser
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Compiler warnings — treat your compiler as a code reviewer
# Java: your IDE/checkstyle does this. In C++ we ask the compiler directly.
if (MSVC)
    add_compile_options(/W4)       # Warning level 4 (high)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

add_executable(order_book
    main.cpp
    order.cpp
    price_level.cpp
    order_book.cpp
)
