cmake_minimum_required(VERSION 3.16)
project(limit_order_book LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(lob_core STATIC
    order.cpp
    trade.cpp
    order_book.cpp
    matching_engine.cpp
)
target_include_directories(lob_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(limit_order_book main.cpp)
target_link_libraries(limit_order_book PRIVATE lob_core)

add_executable(perf_test perf_test.cpp)
target_link_libraries(perf_test PRIVATE lob_core)
