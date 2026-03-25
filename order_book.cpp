#include "order_book.h"

#include <algorithm>
#include <iostream>
#include <iomanip>

namespace {

bool buyLowerPriority(const Order& a, const Order& b) {
    if (a.price() != b.price()) {
        return a.price() < b.price();
    }
    return a.timestamp() > b.timestamp();
}

bool sellLowerPriority(const Order& a, const Order& b) {
    if (a.price() != b.price()) {
        return a.price() > b.price();
    }
    return a.timestamp() > b.timestamp();
}

}  // namespace

bool OrderBook::BuyCmp::operator()(int lhs, int rhs) const {
    const Order& a = orders->at(lhs);
    const Order& b = orders->at(rhs);
    return buyLowerPriority(a, b);
}

bool OrderBook::SellCmp::operator()(int lhs, int rhs) const {
    const Order& a = orders->at(lhs);
    const Order& b = orders->at(rhs);
    return sellLowerPriority(a, b);
}

OrderBook::OrderBook()
    : buyHeap_(BuyCmp{&orders_}),
      sellHeap_(SellCmp{&orders_}) {}

void OrderBook::addOrder(const Order& order) {
    if (orders_.count(order.orderId()) != 0) {
        return;
    }
    orders_.emplace(order.orderId(), order);
    if (order.side() == Side::BUY) {
        buyHeap_.push(order.orderId());
    } else {
        sellHeap_.push(order.orderId());
    }
}

void OrderBook::cancelOrder(int orderId) {
    orders_.erase(orderId);
}

void OrderBook::pruneBuyHeap() {
    while (!buyHeap_.empty()) {
        const int id = buyHeap_.top();
        auto it = orders_.find(id);
        if (it == orders_.end() || it->second.side() != Side::BUY || it->second.quantity() <= 0) {
            buyHeap_.pop();
            continue;
        }
        break;
    }
}

void OrderBook::pruneSellHeap() {
    while (!sellHeap_.empty()) {
        const int id = sellHeap_.top();
        auto it = orders_.find(id);
        if (it == orders_.end() || it->second.side() != Side::SELL || it->second.quantity() <= 0) {
            sellHeap_.pop();
            continue;
        }
        break;
    }
}

void OrderBook::matchOrders() {
    while (true) {
        pruneBuyHeap();
        pruneSellHeap();
        if (buyHeap_.empty() || sellHeap_.empty()) {
            break;
        }

        const int buyId = buyHeap_.top();
        const int sellId = sellHeap_.top();
        Order& buy = orders_.at(buyId);
        Order& sell = orders_.at(sellId);

        if (buy.price() < sell.price()) {
            break;
        }

        const int tradeQty = std::min(buy.quantity(), sell.quantity());

        buy.setQuantity(buy.quantity() - tradeQty);
        sell.setQuantity(sell.quantity() - tradeQty);

        if (buy.quantity() == 0) {
            orders_.erase(buyId);
            buyHeap_.pop();
        }
        if (sell.quantity() == 0) {
            orders_.erase(sellId);
            sellHeap_.pop();
        }
    }
}

void OrderBook::printOrderBook() const {
    std::vector<const Order*> buys;
    std::vector<const Order*> sells;
    buys.reserve(orders_.size());
    sells.reserve(orders_.size());

    for (const auto& entry : orders_) {
        const Order& o = entry.second;
        if (o.side() == Side::BUY) {
            buys.push_back(&o);
        } else {
            sells.push_back(&o);
        }
    }

    std::sort(buys.begin(), buys.end(), [](const Order* a, const Order* b) {
        if (a->price() != b->price()) {
            return a->price() > b->price();
        }
        return a->timestamp() < b->timestamp();
    });
    std::sort(sells.begin(), sells.end(), [](const Order* a, const Order* b) {
        if (a->price() != b->price()) {
            return a->price() < b->price();
        }
        return a->timestamp() < b->timestamp();
    });

    std::cout << std::fixed << std::setprecision(2);
    std::cout << "--- Order book ---\n";
    std::cout << "BUY  (price high -> low):\n";
    for (const Order* o : buys) {
        std::cout << "  id=" << o->orderId() << " price=" << o->price() << " qty=" << o->quantity()
                  << " ts=" << o->timestamp() << '\n';
    }
    std::cout << "SELL (price low -> high):\n";
    for (const Order* o : sells) {
        std::cout << "  id=" << o->orderId() << " price=" << o->price() << " qty=" << o->quantity()
                  << " ts=" << o->timestamp() << '\n';
    }
    std::cout << "------------------\n";
}
