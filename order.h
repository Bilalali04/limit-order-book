#ifndef ORDER_H
#define ORDER_H

enum class Side {
    BUY,
    SELL
};

enum class OrderType {
    LIMIT,
    MARKET
};

class Order {
public:
    Order();
    Order(int orderId, double price, int quantity, Side side, long long timestamp);
    Order(int orderId, double price, int quantity, Side side, long long timestamp, OrderType type);

    int orderId() const;
    void setOrderId(int id);

    double price() const;
    void setPrice(double p);

    int quantity() const;
    void setQuantity(int q);

    Side side() const;
    void setSide(Side s);

    OrderType type() const;
    void setType(OrderType t);

    long long timestamp() const;
    void setTimestamp(long long t);

private:
    int orderId_;
    double price_;
    int quantity_;
    Side side_;
    OrderType type_;
    long long timestamp_;
};

#endif
