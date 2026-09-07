#ifndef SIGNAL_H
#define SIGNAL_H

#include <string>

enum class SignalType
{
    HOLD,
    BUY,
    SELL
};

class Signal
{
private:
    std::string ticker;
    SignalType type;
    double quantity;
    bool active;

public:
    Signal();
    Signal(std::string ticker, SignalType type, double quantity);

    std::string getTicker() const;
    SignalType getType() const;
    double getQuantity() const;
    bool isActive() const;

    void setType(SignalType newType);
    void setQuantity(double newQuantity);

    std::string typeName() const;
};

#endif
