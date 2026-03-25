#include "order_book.h"

#include <algorithm>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>

namespace {

// Max-heap by price (higher is better), then earliest timestamp (smaller is better).
bool buyOrderHasLowerPriority(const Order& a, const Order& b) {
    if (a.price() != b.price()) {
        return a.price() < b.price();
    }
    return a.timestamp() > b.timestamp();
}

// Min-heap by price (lower is better), then earliest timestamp (smaller is better).
bool sellOrderHasLowerPriority(const Order& a, const Order& b) {
    if (a.price() != b.price()) {
        return a.price() > b.price();
    }
    return a.timestamp() > b.timestamp();
}

}  // namespace

bool OrderBook::BuyOrderComparator::operator()(int lhs, int rhs) const {
    const Order& a = orders->at(lhs);
    const Order& b = orders->at(rhs);
    return buyOrderHasLowerPriority(a, b);
}

bool OrderBook::SellOrderComparator::operator()(int lhs, int rhs) const {
    const Order& a = orders->at(lhs);
    const Order& b = orders->at(rhs);
    return sellOrderHasLowerPriority(a, b);
}

OrderBook::OrderBook()
    : buyHeap_(BuyOrderComparator{&orders_}),
      sellHeap_(SellOrderComparator{&orders_}),
      ordersLog_("orders.log", std::ios::app),
      tradesLog_("trades.log", std::ios::app) {}

void OrderBook::logOrder(const Order& order) {
    if (!ordersLog_.is_open()) {
        return;
    }
    ordersLog_ << "timestamp=" << order.timestamp()
               << ", orderId=" << order.orderId()
               << ", price=" << std::fixed << std::setprecision(2) << order.price()
               << ", quantity=" << order.quantity()
               << '\n';
}

void OrderBook::logTrade(const Trade& trade) {
    if (!tradesLog_.is_open()) {
        return;
    }
    tradesLog_ << "timestamp=" << trade.timestamp()
               << ", orderId=" << trade.buyOrderId() << "/" << trade.sellOrderId()
               << ", price=" << std::fixed << std::setprecision(2) << trade.price()
               << ", quantity=" << trade.quantity()
               << '\n';
}

long long OrderBook::nowNs() {
    return std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
}

void OrderBook::recordLatencyForOrder(int orderId, long long tradeExecNs) {
    const auto it = orderAddTimesNs_.find(orderId);
    if (it == orderAddTimesNs_.end()) {
        return;
    }
    const long long deltaNs = tradeExecNs - it->second;
    if (deltaNs < 0) {
        return;
    }
    const double latencyMs = static_cast<double>(deltaNs) / 1'000'000.0;
    latencySumMs_ += latencyMs;
    latencyMinMs_ = std::min(latencyMinMs_, latencyMs);
    latencyMaxMs_ = std::max(latencyMaxMs_, latencyMs);
    ++latencySamples_;
}

void OrderBook::addOrder(const Order& order) {
    const int id = order.orderId();
    if (orders_.count(id) != 0) {
        return;
    }
    orders_.emplace(id, order);
    orderAddTimesNs_[id] = nowNs();
    logOrder(order);
    if (order.type() == OrderType::MARKET) {
        executeMarketOrder(id);
        return;
    }
    if (order.side() == Side::BUY) {
        buyHeap_.push(id);
    } else if (order.side() == Side::SELL) {
        sellHeap_.push(id);
    }
}

void OrderBook::executeMarketOrder(int orderId) {
    auto marketIt = orders_.find(orderId);
    if (marketIt == orders_.end()) {
        return;
    }
    Order& marketOrder = marketIt->second;

    if (marketOrder.side() == Side::BUY) {
        while (marketOrder.quantity() > 0) {
            pruneSellHeap();
            if (sellHeap_.empty()) {
                break;
            }

            const int sellId = sellHeap_.top();
            Order& sell = orders_.at(sellId);
            const int tradeQty = std::min(marketOrder.quantity(), sell.quantity());
            const double tradePrice = sell.price();
            const long long tradeTs = std::max(marketOrder.timestamp(), sell.timestamp());
            trades_.emplace_back(orderId, sellId, tradePrice, tradeQty, tradeTs);
            logTrade(trades_.back());
            const long long execNs = nowNs();
            recordLatencyForOrder(orderId, execNs);
            recordLatencyForOrder(sellId, execNs);

            marketOrder.setQuantity(marketOrder.quantity() - tradeQty);
            sell.setQuantity(sell.quantity() - tradeQty);
            if (sell.quantity() == 0) {
                orders_.erase(sellId);
                orderAddTimesNs_.erase(sellId);
                sellHeap_.pop();
            }
        }
    } else if (marketOrder.side() == Side::SELL) {
        while (marketOrder.quantity() > 0) {
            pruneBuyHeap();
            if (buyHeap_.empty()) {
                break;
            }

            const int buyId = buyHeap_.top();
            Order& buy = orders_.at(buyId);
            const int tradeQty = std::min(marketOrder.quantity(), buy.quantity());
            const double tradePrice = buy.price();
            const long long tradeTs = std::max(marketOrder.timestamp(), buy.timestamp());
            trades_.emplace_back(buyId, orderId, tradePrice, tradeQty, tradeTs);
            logTrade(trades_.back());
            const long long execNs = nowNs();
            recordLatencyForOrder(buyId, execNs);
            recordLatencyForOrder(orderId, execNs);

            marketOrder.setQuantity(marketOrder.quantity() - tradeQty);
            buy.setQuantity(buy.quantity() - tradeQty);
            if (buy.quantity() == 0) {
                orders_.erase(buyId);
                orderAddTimesNs_.erase(buyId);
                buyHeap_.pop();
            }
        }
    }

    // Market orders are IOC-style: any remainder is discarded.
    orders_.erase(orderId);
    orderAddTimesNs_.erase(orderId);
}

void OrderBook::rebuildBuyHeap() {
    std::priority_queue<int, std::vector<int>, BuyOrderComparator> heap(BuyOrderComparator{&orders_});
    for (const auto& entry : orders_) {
        if (entry.second.side() == Side::BUY) {
            heap.push(entry.first);
        }
    }
    buyHeap_ = std::move(heap);
}

void OrderBook::rebuildSellHeap() {
    std::priority_queue<int, std::vector<int>, SellOrderComparator> heap(SellOrderComparator{&orders_});
    for (const auto& entry : orders_) {
        if (entry.second.side() == Side::SELL) {
            heap.push(entry.first);
        }
    }
    sellHeap_ = std::move(heap);
}

void OrderBook::cancelOrder(int orderId) {
    const auto it = orders_.find(orderId);
    if (it == orders_.end()) {
        return;
    }
    const Side side = it->second.side();
    orders_.erase(it);
    orderAddTimesNs_.erase(orderId);
    if (side == Side::BUY) {
        rebuildBuyHeap();
    } else if (side == Side::SELL) {
        rebuildSellHeap();
    }
}

void OrderBook::modifyOrder(int orderId, double newPrice, int newQuantity) {
    const auto it = orders_.find(orderId);
    if (it == orders_.end()) {
        return;
    }
    const Side side = it->second.side();
    if (newQuantity <= 0) {
        orders_.erase(it);
        orderAddTimesNs_.erase(orderId);
        if (side == Side::BUY) {
            rebuildBuyHeap();
        } else if (side == Side::SELL) {
            rebuildSellHeap();
        }
        return;
    }
    it->second.setPrice(newPrice);
    it->second.setQuantity(newQuantity);
    if (side == Side::BUY) {
        rebuildBuyHeap();
    } else if (side == Side::SELL) {
        rebuildSellHeap();
    }
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
    // While best bid >= best ask: cross and record; otherwise stop.
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
        const double tradePrice = sell.price();
        const long long tradeTs = std::max(buy.timestamp(), sell.timestamp());
        trades_.emplace_back(buyId, sellId, tradePrice, tradeQty, tradeTs);
        logTrade(trades_.back());
        const long long execNs = nowNs();
        recordLatencyForOrder(buyId, execNs);
        recordLatencyForOrder(sellId, execNs);

        buy.setQuantity(buy.quantity() - tradeQty);
        sell.setQuantity(sell.quantity() - tradeQty);

        if (buy.quantity() == 0) {
            orders_.erase(buyId);
            orderAddTimesNs_.erase(buyId);
            buyHeap_.pop();
        }
        if (sell.quantity() == 0) {
            orders_.erase(sellId);
            orderAddTimesNs_.erase(sellId);
            sellHeap_.pop();
        }
    }
}

OrderBook::LatencyStats OrderBook::latencyStats() const {
    LatencyStats stats;
    stats.samples = latencySamples_;
    if (latencySamples_ == 0) {
        return stats;
    }
    stats.averageMs = latencySumMs_ / static_cast<double>(latencySamples_);
    stats.minMs = latencyMinMs_;
    stats.maxMs = latencyMaxMs_;
    return stats;
}

void OrderBook::clearTrades() {
    trades_.clear();
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
        } else if (o.side() == Side::SELL) {
            sells.push_back(&o);
        }
    }

    std::sort(buys.begin(), buys.end(), [](const Order* a, const Order* b) {
        return buyOrderHasLowerPriority(*b, *a);
    });
    std::sort(sells.begin(), sells.end(), [](const Order* a, const Order* b) {
        return sellOrderHasLowerPriority(*b, *a);
    });

    const std::ios::fmtflags oldFlags = std::cout.flags();
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "--- Order book ---\n";
    if (orders_.empty()) {
        std::cout << "No open orders.\n";
        std::cout << "------------------\n";
        std::cout.flags(oldFlags);
        return;
    }

    std::cout << "BUY  (price high -> low, then earlier time):\n";
    if (buys.empty()) {
        std::cout << "  (none)\n";
    } else {
        std::cout << "  " << std::setw(6) << "id" << std::setw(12) << "price" << std::setw(8) << "qty"
                  << std::setw(14) << "timestamp" << '\n';
        for (const Order* o : buys) {
            std::cout << "  " << std::setw(6) << o->orderId() << std::setw(12) << o->price() << std::setw(8)
                      << o->quantity() << std::setw(14) << o->timestamp() << '\n';
        }
    }

    std::cout << "SELL (price low -> high, then earlier time):\n";
    if (sells.empty()) {
        std::cout << "  (none)\n";
    } else {
        std::cout << "  " << std::setw(6) << "id" << std::setw(12) << "price" << std::setw(8) << "qty"
                  << std::setw(14) << "timestamp" << '\n';
        for (const Order* o : sells) {
            std::cout << "  " << std::setw(6) << o->orderId() << std::setw(12) << o->price() << std::setw(8)
                      << o->quantity() << std::setw(14) << o->timestamp() << '\n';
        }
    }
    std::cout << "------------------\n";
    std::cout.flags(oldFlags);
}

void OrderBook::saveOrderBookToFile(const std::string& filename) const {
    std::ofstream out(filename);
    if (!out.is_open()) {
        return;
    }

    std::unordered_map<double, int> buyLevels;
    std::unordered_map<double, int> sellLevels;
    buyLevels.reserve(orders_.size());
    sellLevels.reserve(orders_.size());

    for (const auto& entry : orders_) {
        const Order& o = entry.second;
        if (o.side() == Side::BUY) {
            buyLevels[o.price()] += o.quantity();
        } else if (o.side() == Side::SELL) {
            sellLevels[o.price()] += o.quantity();
        }
    }

    std::vector<std::pair<double, int>> buyRows;
    std::vector<std::pair<double, int>> sellRows;
    buyRows.reserve(buyLevels.size());
    sellRows.reserve(sellLevels.size());

    for (const auto& level : buyLevels) {
        buyRows.push_back(level);
    }
    for (const auto& level : sellLevels) {
        sellRows.push_back(level);
    }

    std::sort(buyRows.begin(), buyRows.end(), [](const std::pair<double, int>& a, const std::pair<double, int>& b) {
        return a.first > b.first;
    });
    std::sort(sellRows.begin(), sellRows.end(), [](const std::pair<double, int>& a, const std::pair<double, int>& b) {
        return a.first < b.first;
    });

    out << std::fixed << std::setprecision(2);
    out << "BUY ORDERS:\n";
    out << "Price | Total Quantity\n";
    for (const auto& row : buyRows) {
        out << row.first << " | " << row.second << '\n';
    }
    out << '\n';
    out << "SELL ORDERS:\n";
    out << "Price | Total Quantity\n";
    for (const auto& row : sellRows) {
        out << row.first << " | " << row.second << '\n';
    }
}

void OrderBook::saveTradesToFile(const std::string& filename) const {
    std::ofstream out(filename);
    if (!out.is_open()) {
        return;
    }

    out << std::fixed << std::setprecision(2);
    out << "TradeID | BuyOrderID | SellOrderID | Price | Quantity | Timestamp\n";
    for (std::size_t i = 0; i < trades_.size(); ++i) {
        const Trade& t = trades_[i];
        const std::size_t tradeId = i + 1;
        out << tradeId
            << " | " << t.buyOrderId()
            << " | " << t.sellOrderId()
            << " | " << t.price()
            << " | " << t.quantity()
            << " | " << t.timestamp()
            << '\n';
    }
}
