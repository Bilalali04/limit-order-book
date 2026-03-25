#include "trade.h"

Trade::Trade(int buyOrderId, int sellOrderId, double price, int quantity, long long timestamp)
    : buyOrderId_(buyOrderId),
      sellOrderId_(sellOrderId),
      price_(price),
      quantity_(quantity),
      timestamp_(timestamp) {}

int Trade::buyOrderId() const {
    return buyOrderId_;
}

int Trade::sellOrderId() const {
    return sellOrderId_;
}

double Trade::price() const {
    return price_;
}

int Trade::quantity() const {
    return quantity_;
}

long long Trade::timestamp() const {
    return timestamp_;
}
