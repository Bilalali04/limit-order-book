#include "matching_engine.h"
#include "trade.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

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

bool loadOrdersFromCsv(const std::string& path, MatchingEngine* engine) {
    std::ifstream in(path);
    if (!in.is_open()) {
        std::cerr << "Failed to open CSV file: " << path << '\n';
        return false;
    }

    std::string line;
    int orderId = 1;
    long long timestamp = 1;
    while (std::getline(in, line)) {
        if (line.empty()) {
            continue;
        }

        std::stringstream ss(line);
        std::string sideToken;
        std::string priceToken;
        std::string qtyToken;

        if (!std::getline(ss, sideToken, ',') ||
            !std::getline(ss, priceToken, ',') ||
            !std::getline(ss, qtyToken, ',')) {
            std::cerr << "Skipping invalid line: " << line << '\n';
            continue;
        }

        Side side;
        if (!parseSide(sideToken, &side)) {
            std::cerr << "Skipping line with invalid side: " << line << '\n';
            continue;
        }

        try {
            const double price = std::stod(priceToken);
            const int quantity = std::stoi(qtyToken);
            if (quantity <= 0) {
                std::cerr << "Skipping line with non-positive quantity: " << line << '\n';
                continue;
            }
            engine->addOrder(Order(orderId, price, quantity, side, timestamp, OrderType::LIMIT));
            ++orderId;
            ++timestamp;
        } catch (const std::exception&) {
            std::cerr << "Skipping line with invalid numeric values: " << line << '\n';
        }
    }

    return true;
}

}  // namespace

int main(int argc, char* argv[]) {
    const std::string csvPath = (argc > 1) ? argv[1] : "orders.csv";
    MatchingEngine engine;
    if (!loadOrdersFromCsv(csvPath, &engine)) {
        return 1;
    }
    engine.matchOrders();

    std::cout << "Trades:\n";
    const std::vector<Trade>& trades = engine.trades();
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
    engine.printOrderBook();

    return 0;
}
