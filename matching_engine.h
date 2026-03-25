#ifndef MATCHING_ENGINE_H
#define MATCHING_ENGINE_H

#include "order_book.h"

#include <string>
#include <vector>

class MatchingEngine {
public:
    void addOrder(const Order& order);
    void cancelOrder(int orderId);
    void modifyOrder(int orderId, double newPrice, int newQuantity);
    void matchOrders();
    void printOrderBook() const;
    void saveOrderBookToFile(const std::string& filename) const;
    void saveTradesToFile(const std::string& filename) const;
    OrderBook::LatencyStats latencyStats() const;

    const std::vector<Trade>& trades() const;
    void clearTrades();

    OrderBook& orderBook();
    const OrderBook& orderBook() const;

private:
    OrderBook orderBook_;
};

#endif
