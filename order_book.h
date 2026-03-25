#ifndef ORDER_BOOK_H
#define ORDER_BOOK_H

#include <queue>
#include <unordered_map>
#include <vector>

#include "order.h"

class OrderBook {
public:
    OrderBook();

    void addOrder(const Order& order);
    void cancelOrder(int orderId);
    void matchOrders();
    void printOrderBook() const;

private:
    struct BuyCmp {
        std::unordered_map<int, Order>* orders;
        bool operator()(int lhs, int rhs) const;
    };

    struct SellCmp {
        std::unordered_map<int, Order>* orders;
        bool operator()(int lhs, int rhs) const;
    };

    void pruneBuyHeap();
    void pruneSellHeap();

    std::unordered_map<int, Order> orders_;
    std::priority_queue<int, std::vector<int>, BuyCmp> buyHeap_;
    std::priority_queue<int, std::vector<int>, SellCmp> sellHeap_;
};

#endif
