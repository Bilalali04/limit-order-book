#include "matching_engine.h"

#include <chrono>
#include <iostream>
#include <vector>
#include <random>

namespace {

struct Result {
    double totalSeconds = 0.0;
    double ordersPerSecond = 0.0;
    std::size_t trades = 0;
    OrderBook::LatencyStats latency;
};

Result run(int orderCount) {
    // Pre-generate random order parameters so timing focuses on the engine.
    std::mt19937 rng(42);
    std::uniform_real_distribution<double> priceDist(90.0, 110.0);
    std::uniform_int_distribution<int> qtyDist(1, 100);
    std::bernoulli_distribution sideDist(0.5);

    std::vector<double> prices(orderCount);
    std::vector<int> quantities(orderCount);
    std::vector<Side> sides(orderCount);

    for (int i = 0; i < orderCount; ++i) {
        prices[i] = priceDist(rng);
        quantities[i] = qtyDist(rng);
        sides[i] = sideDist(rng) ? Side::BUY : Side::SELL;
    }

    MatchingEngine engine;

    const auto tStart = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < orderCount; ++i) {
        const int id = i + 1;
        engine.addOrder(Order(id, prices[i], quantities[i], sides[i], static_cast<long long>(id)));
    }
    engine.matchOrders();
    const auto tEnd = std::chrono::high_resolution_clock::now();

    const double totalSeconds =
        std::chrono::duration<double>(tEnd - tStart).count();

    Result r;
    r.totalSeconds = totalSeconds;
    r.ordersPerSecond = (totalSeconds > 0.0)
        ? static_cast<double>(orderCount) / totalSeconds
        : 0.0;
    r.trades = engine.trades().size();
    r.latency = engine.latencyStats();
    return r;
}

}  // namespace

int main() {
    const std::vector<int> scenarios = {100'000, 1'000'000};

    for (int orderCount : scenarios) {
        std::cout << "=== Scenario: " << orderCount << " random orders ===\n";
        Result r = run(orderCount);
        std::cout << "  Total execution time: " << r.totalSeconds << " s\n";
        std::cout << "  Orders processed per second: " << r.ordersPerSecond << '\n';
        std::cout << "  Trades recorded: " << r.trades << '\n';
        if (r.latency.samples == 0) {
            std::cout << "  Average latency: N/A\n";
            std::cout << "  Maximum latency: N/A\n";
            std::cout << "  Minimum latency: N/A\n\n";
        } else {
            std::cout << "  Average latency: " << r.latency.averageMs << " ms\n";
            std::cout << "  Maximum latency: " << r.latency.maxMs << " ms\n";
            std::cout << "  Minimum latency: " << r.latency.minMs << " ms\n\n";
        }
    }

    return 0;
}
