#include "matching_engine.h"

#include <chrono>
#include <iostream>
#include <random>

int main() {
    constexpr int kOrders = 100'000;

    MatchingEngine engine;
    std::mt19937 rng(42);
    std::uniform_real_distribution<double> priceDist(90.0, 110.0);
    std::uniform_int_distribution<int> qtyDist(1, 100);
    std::bernoulli_distribution sideDist(0.5);

    const auto tAddStart = std::chrono::high_resolution_clock::now();
    for (int i = 1; i <= kOrders; ++i) {
        const Side side = sideDist(rng) ? Side::BUY : Side::SELL;
        engine.addOrder(
            Order(i, priceDist(rng), qtyDist(rng), side, static_cast<long long>(i)));
    }
    const auto tAddEnd = std::chrono::high_resolution_clock::now();

    const auto tMatchStart = std::chrono::high_resolution_clock::now();
    engine.matchOrders();
    const auto tMatchEnd = std::chrono::high_resolution_clock::now();

    const double addMs =
        std::chrono::duration<double, std::milli>(tAddEnd - tAddStart).count();
    const double matchMs =
        std::chrono::duration<double, std::milli>(tMatchEnd - tMatchStart).count();
    const double totalMs =
        std::chrono::duration<double, std::milli>(tMatchEnd - tAddStart).count();

    std::cout << "Random order simulation (" << kOrders << " orders)\n";
    std::cout << "  addOrder (total): " << addMs << " ms\n";
    std::cout << "  matchOrders:      " << matchMs << " ms\n";
    std::cout << "  wall total:       " << totalMs << " ms\n";
    std::cout << "  trades recorded: " << engine.trades().size() << '\n';

    return 0;
}
