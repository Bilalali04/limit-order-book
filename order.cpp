#include "order.h"

Order::Order()
    : orderId_(0), price_(0.0), quantity_(0), side_(Side::BUY), timestamp_(0) {}

Order::Order(int orderId, double price, int quantity, Side side, long long timestamp)
    : orderId_(orderId), price_(price), quantity_(quantity), side_(side), timestamp_(timestamp) {}

int Order::orderId() const {
    return orderId_;
}

void Order::setOrderId(int id) {
    orderId_ = id;
}

double Order::price() const {
    return price_;
}

void Order::setPrice(double p) {
    price_ = p;
}

int Order::quantity() const {
    return quantity_;
}

void Order::setQuantity(int q) {
    quantity_ = q;
}

Side Order::side() const {
    return side_;
}

void Order::setSide(Side s) {
    side_ = s;
}

long long Order::timestamp() const {
    return timestamp_;
}

void Order::setTimestamp(long long t) {
    timestamp_ = t;
}
