#ifndef ORDER_BOOK_H
#define ORDER_BOOK_H

#include <queue>
#include <unordered_map>
#include <vector>

#include "order.h"
#include "trade.h"

class OrderBook {
public:
    OrderBook();

    void addOrder(const Order& order);
    void cancelOrder(int orderId);
    void matchOrders();
    void printOrderBook() const;

    const std::vector<Trade>& trades() const { return trades_; }
    void clearTrades();

private:
    // priority_queue Compare: returns true if lhs has lower priority than rhs (top = highest priority).
    // Heaps store orderId; lookup in orders_ for current price/timestamp (partial fills stay correct).

    struct BuyOrderComparator {
        std::unordered_map<int, Order>* orders;
        bool operator()(int lhs, int rhs) const;
    };

    struct SellOrderComparator {
        std::unordered_map<int, Order>* orders;
        bool operator()(int lhs, int rhs) const;
    };

    void pruneBuyHeap();
    void pruneSellHeap();
    void rebuildBuyHeap();
    void rebuildSellHeap();

    std::unordered_map<int, Order> orders_;
    std::priority_queue<int, std::vector<int>, BuyOrderComparator> buyHeap_;
    std::priority_queue<int, std::vector<int>, SellOrderComparator> sellHeap_;
    std::vector<Trade> trades_;
};

#endif
