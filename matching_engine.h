#ifndef MATCHING_ENGINE_H
#define MATCHING_ENGINE_H

#include "order_book.h"

#include <vector>

class MatchingEngine {
public:
    void addOrder(const Order& order);
    void cancelOrder(int orderId);
    void modifyOrder(int orderId, double newPrice, int newQuantity);
    void matchOrders();
    void printOrderBook() const;

    const std::vector<Trade>& trades() const;
    void clearTrades();

    OrderBook& orderBook();
    const OrderBook& orderBook() const;

private:
    OrderBook orderBook_;
};

#endif
