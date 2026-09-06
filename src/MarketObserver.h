#ifndef MARKETOBSERVER_H
#define MARKETOBSERVER_H

#include <string>

class MarketObserver
{
public:
    virtual ~MarketObserver() = default;

    virtual void onPriceUpdate(std::string ticker, double price) = 0;
};

#endif
