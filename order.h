#ifndef ORDER_H
#define ORDER_H

enum class Side {
    BUY,
    SELL
};

class Order {
public:
    Order();
    Order(int orderId, double price, int quantity, Side side, long long timestamp);

    int orderId() const;
    void setOrderId(int id);

    double price() const;
    void setPrice(double p);

    int quantity() const;
    void setQuantity(int q);

    Side side() const;
    void setSide(Side s);

    long long timestamp() const;
    void setTimestamp(long long t);

private:
    int orderId_;
    double price_;
    int quantity_;
    Side side_;
    long long timestamp_;
};

#endif
