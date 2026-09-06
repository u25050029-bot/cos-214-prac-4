#ifndef MARKETSUBJECT_H
#define MARKETSUBJECT_H

#include <string>
#include "MarketObserver.h"

class MarketSubject
{
public:
    virtual ~MarketSubject() = default;

    virtual void attach(MarketObserver *observer, std::string ticker) = 0;
    virtual void detach(MarketObserver *observer, std::string ticker) = 0;
    virtual void notify(std::string ticker, double price) = 0;
};

#endif
