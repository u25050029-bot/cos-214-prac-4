#include "Signal.h"

Signal::Signal()
{
    this->ticker = "";
    this->type = SignalType::HOLD;
    this->quantity = 0.0;
    this->active = false;
}

Signal::Signal(std::string ticker, SignalType type, double quantity)
{
    this->ticker = ticker;
    this->type = type;
    this->quantity = quantity;
    this->active = true;
}

std::string Signal::getTicker() const
{
    return ticker;
}

SignalType Signal::getType() const
{
    return type;
}

double Signal::getQuantity() const
{
    return quantity;
}

bool Signal::isActive() const
{
    return active;
}

void Signal::setType(SignalType newType)
{
    type = newType;
}

void Signal::setQuantity(double newQuantity)
{
    quantity = newQuantity;
}

std::string Signal::typeName() const
{
    switch (type)
    {
    case SignalType::BUY:
        return "BUY";
    case SignalType::SELL:
        return "SELL";
    default:
        return "HOLD";
    }
}
