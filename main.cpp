#include "matching_engine.h"
#include "trade.h"

#include <condition_variable>
#include <deque>
#include <fstream>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

namespace {

bool parseSide(const std::string& token, Side* side) {
    if (token == "BUY") {
        *side = Side::BUY;
        return true;
    }
    if (token == "SELL") {
        *side = Side::SELL;
        return true;
    }
    return false;
}

bool parseOrderCsvLine(
    const std::string& line, int orderId, long long timestamp, Order* outOrder) {
    std::stringstream ss(line);
    std::string sideToken;
    std::string priceToken;
    std::string qtyToken;

    if (!std::getline(ss, sideToken, ',') ||
        !std::getline(ss, priceToken, ',') ||
        !std::getline(ss, qtyToken, ',')) {
        return false;
    }

    Side side;
    if (!parseSide(sideToken, &side)) {
        return false;
    }

    try {
        const double price = std::stod(priceToken);
        const int quantity = std::stoi(qtyToken);
        if (quantity <= 0) {
            return false;
        }
        *outOrder = Order(orderId, price, quantity, side, timestamp, OrderType::LIMIT);
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

void inputThreadFunc(
    const std::string& path,
    std::deque<Order>* orderQueue,
    std::mutex* orderQueueMutex,
    std::condition_variable* orderCv,
    bool* inputDone) {
    std::ifstream in(path);
    if (!in.is_open()) {
        std::cerr << "Failed to open CSV file: " << path << '\n';
        {
            std::lock_guard<std::mutex> lock(*orderQueueMutex);
            *inputDone = true;
        }
        orderCv->notify_all();
        return;
    }

    std::string line;
    int orderId = 1;
    long long timestamp = 1;
    while (std::getline(in, line)) {
        if (line.empty()) {
            continue;
        }
        Order parsed;
        if (!parseOrderCsvLine(line, orderId, timestamp, &parsed)) {
            std::cerr << "Skipping line with invalid numeric values: " << line << '\n';
            continue;
        }

        {
            std::lock_guard<std::mutex> lock(*orderQueueMutex);
            orderQueue->push_back(parsed);
        }
        orderCv->notify_one();
        ++orderId;
        ++timestamp;
    }

    {
        std::lock_guard<std::mutex> lock(*orderQueueMutex);
        *inputDone = true;
    }
    orderCv->notify_all();
}

}  // namespace

int main(int argc, char* argv[]) {
    const std::string csvPath = (argc > 1) ? argv[1] : "orders.csv";
    MatchingEngine engine;
    std::deque<Order> orderQueue;
    std::deque<Trade> tradeQueue;
    std::mutex orderQueueMutex;
    std::mutex tradeQueueMutex;
    std::mutex engineMutex;
    std::condition_variable orderCv;
    std::condition_variable tradeCv;
    bool inputDone = false;
    bool matchingDone = false;

    std::thread inputThread(
        inputThreadFunc,
        csvPath,
        &orderQueue,
        &orderQueueMutex,
        &orderCv,
        &inputDone);

    std::thread matchingThread([&]() {
        std::size_t emittedTrades = 0;
        while (true) {
            Order nextOrder;
            bool hasOrder = false;
            {
                std::unique_lock<std::mutex> lock(orderQueueMutex);
                orderCv.wait(lock, [&]() {
                    return inputDone || !orderQueue.empty();
                });
                if (!orderQueue.empty()) {
                    nextOrder = orderQueue.front();
                    orderQueue.pop_front();
                    hasOrder = true;
                } else if (inputDone) {
                    break;
                }
            }

            if (hasOrder) {
                std::lock_guard<std::mutex> engineLock(engineMutex);
                engine.addOrder(nextOrder);
                engine.matchOrders();
                const std::vector<Trade>& allTrades = engine.trades();
                if (allTrades.size() > emittedTrades) {
                    std::lock_guard<std::mutex> tradeLock(tradeQueueMutex);
                    for (std::size_t i = emittedTrades; i < allTrades.size(); ++i) {
                        tradeQueue.push_back(allTrades[i]);
                    }
                    emittedTrades = allTrades.size();
                    tradeCv.notify_one();
                }
            }
        }

        {
            std::lock_guard<std::mutex> lock(tradeQueueMutex);
            matchingDone = true;
        }
        tradeCv.notify_all();
    });

    std::thread tradeLoggingThread([&]() {
        std::ofstream out("trades.log", std::ios::app);
        if (!out.is_open()) {
            std::cerr << "Failed to open trades.log\n";
            return;
        }
        while (true) {
            Trade t(0, 0, 0.0, 0, 0);
            bool hasTrade = false;
            {
                std::unique_lock<std::mutex> lock(tradeQueueMutex);
                tradeCv.wait(lock, [&]() {
                    return matchingDone || !tradeQueue.empty();
                });
                if (!tradeQueue.empty()) {
                    t = tradeQueue.front();
                    tradeQueue.pop_front();
                    hasTrade = true;
                } else if (matchingDone) {
                    break;
                }
            }
            if (hasTrade) {
                out << "timestamp=" << t.timestamp()
                    << ", orderId=" << t.buyOrderId() << "/" << t.sellOrderId()
                    << ", price=" << t.price()
                    << ", quantity=" << t.quantity()
                    << '\n';
            }
        }
    });

    inputThread.join();
    matchingThread.join();
    tradeLoggingThread.join();

    std::cout << "Trades:\n";
    std::vector<Trade> trades;
    {
        std::lock_guard<std::mutex> engineLock(engineMutex);
        trades = engine.trades();
    }
    if (trades.empty()) {
        std::cout << "  (none)\n";
    } else {
        for (const Trade& t : trades) {
            std::cout << "  buyOrderId=" << t.buyOrderId() << " sellOrderId=" << t.sellOrderId()
                      << " price=" << t.price() << " qty=" << t.quantity() << " ts=" << t.timestamp()
                      << '\n';
        }
    }

    std::cout << '\n';
    {
        std::lock_guard<std::mutex> engineLock(engineMutex);
        engine.printOrderBook();
    }

    return 0;
}
