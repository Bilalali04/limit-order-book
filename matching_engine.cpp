#include "matching_engine.h"

void MatchingEngine::addOrder(const Order& order) {
    orderBook_.addOrder(order);
}

void MatchingEngine::cancelOrder(int orderId) {
    orderBook_.cancelOrder(orderId);
}

void MatchingEngine::modifyOrder(int orderId, double newPrice, int newQuantity) {
    orderBook_.modifyOrder(orderId, newPrice, newQuantity);
}

void MatchingEngine::matchOrders() {
    orderBook_.matchOrders();
}

void MatchingEngine::printOrderBook() const {
    orderBook_.printOrderBook();
}

const std::vector<Trade>& MatchingEngine::trades() const {
    return orderBook_.trades();
}

void MatchingEngine::clearTrades() {
    orderBook_.clearTrades();
}

OrderBook& MatchingEngine::orderBook() {
    return orderBook_;
}

const OrderBook& MatchingEngine::orderBook() const {
    return orderBook_;
}
