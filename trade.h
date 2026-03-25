#ifndef TRADE_H
#define TRADE_H

class Trade {
public:
    Trade(int buyOrderId, int sellOrderId, double price, int quantity, long long timestamp);

    int buyOrderId() const;
    int sellOrderId() const;
    double price() const;
    int quantity() const;
    long long timestamp() const;

private:
    int buyOrderId_;
    int sellOrderId_;
    double price_;
    int quantity_;
    long long timestamp_;
};

#endif
