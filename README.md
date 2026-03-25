# Limit Order Book Matching Engine (C++)

## Overview
This project implements a limit order book and trade matching engine similar to those used in electronic financial exchanges. The system processes buy and sell orders, matches orders using price-time priority, executes trades, and maintains the state of the order book.

The project demonstrates how trading systems match orders efficiently using appropriate data structures and low-latency matching logic.

---

## Features
- Limit order support
- Market order support
- Order matching using price-time priority
- Order cancellation
- Order modification
- Trade execution and trade history tracking
- Order book snapshot export (aggregated by price level)
- Performance testing with high-volume simulated orders
- Latency measurement for order execution
- Built using C++ with CMake build system

---

## What is a Limit Order Book?
A limit order book stores buy and sell orders for a financial asset.

- Buy orders (bids): Orders from traders who want to buy at a specific price.
- Sell orders (asks): Orders from traders who want to sell at a specific price.

Orders remain in the order book until:
- They are matched with another order
- They are cancelled
- They are modified

A trade occurs when a buy price is greater than or equal to a sell price.

---

## Price-Time Priority
This matching engine uses price-time priority, which is the standard matching rule used in most exchanges.

Priority is determined by:
1. Better price first
   - Higher price buy orders get priority
   - Lower price sell orders get priority
2. Earlier order first
   - If two orders have the same price, the order that arrived first is executed first

---

## Data Structures Used

| Data Structure           | Purpose                                     |
|--------------------------|---------------------------------------------|
| Max Heap                 | Stores buy orders (highest price first)     |
| Min Heap                 | Stores sell orders (lowest price first)     |
| Hash Map (unordered_map) | Fast order lookup for cancel/modify         |
| Vector                   | Stores executed trades                      |

These data structures allow the system to maintain efficient order matching and low-latency performance.

---

## Matching Algorithm

The matching engine continuously checks whether the highest buy order can match the lowest sell order.

Matching Rule:

While highest buy price >= lowest sell price:
    Execute trade
    Trade quantity = min(buy quantity, sell quantity)
    Reduce quantities
    Remove completed orders
    Record trade

Trades are executed at the sell price.

---

## Time Complexity

| Operation         | Complexity |
|-------------------|------------|
| Add Order         | O(log n)   |
| Cancel Order      | O(1)       |
| Modify Order      | O(log n)   |
| Match Order       | O(log n)   |
| Get Best Buy/Sell | O(1)       |

---

## Project Structure

```
limit-order-book
│
├── main.cpp
├── order.h
├── order.cpp
├── order_book.h
├── order_book.cpp
├── matching_engine.h
├── matching_engine.cpp
├── trade.h
├── trade.cpp
├── perf_test.cpp
├── CMakeLists.txt
├── README.md
```

---

## How to Compile and Run

### Using CMake:
```
mkdir build
cd build
cmake ..
make
./engine
```

### Using g++:
```
g++ main.cpp order.cpp order_book.cpp matching_engine.cpp trade.cpp -o engine
./engine
```

---

## Performance Test
The performance test simulates a large number of buy and sell orders to evaluate the efficiency of the matching engine.

The system measures:
- Execution time
- Orders processed per second
- Average latency per order

This demonstrates the scalability and performance of the matching engine under high order volume.

---

## Order Book Snapshot
The system can export the order book to a file. The snapshot aggregates orders by price level and shows total quantity at each price.

Example format:

BUY ORDERS:
Price | Quantity
100   | 12
99    | 5

SELL ORDERS:
Price | Quantity
101   | 8
102   | 6

---

## Trade History
All executed trades can be exported to a file with the following format:

TradeID | BuyOrderID | SellOrderID | Price | Quantity | Timestamp

---

## Example Output

Trade Executed:
Buy Order 4 matched with Sell Order 2 at Price 99 for Quantity 3

Trade Executed:
Buy Order 1 matched with Sell Order 2 at Price 99 for Quantity 2

Remaining Order Book:

BUY ORDERS:
Price | Quantity
100   | 8

SELL ORDERS:
Price | Quantity
101   | 8

---

## Purpose of This Project
This project demonstrates:
- Data structures (heaps, hash maps)
- Object-oriented programming in C++
- Algorithm design
- System design
- Low-latency matching logic
- Performance benchmarking

This type of system is commonly used in:
- Electronic trading systems
- Stock exchanges
- Cryptocurrency exchanges
- High-frequency trading systems

---

## Future Improvements
- Multithreaded order processing
- Networked order input
- REST API for submitting orders
- Persistent order storage
- Real-time market data simulation