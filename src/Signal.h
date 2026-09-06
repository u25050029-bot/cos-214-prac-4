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
    std::string timestamp;
    double quantity;
    bool active;

public:
    Signal();
    Signal(std::string ticker, SignalType type, std::string timestamp, double quantity);

    std::string getTicker() const;
    SignalType getType() const;
    std::string getTimestamp() const;
    double getQuantity() const;
    bool isActive() const;

    void setType(SignalType newType);
    void setQuantity(double newQuantity);
    void setActive(bool newActive);

    std::string typeName() const;
};

#endif
