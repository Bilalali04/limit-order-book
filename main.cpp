#include "matching_engine.h"
#include "trade.h"

#include <iostream>

int main() {
    MatchingEngine engine;

    engine.addOrder(Order(1, 100.0, 10, Side::BUY, 1));
    engine.addOrder(Order(2, 99.0, 5, Side::SELL, 2));
    engine.addOrder(Order(3, 101.0, 8, Side::SELL, 3));
    engine.addOrder(Order(4, 102.0, 3, Side::BUY, 4));

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
