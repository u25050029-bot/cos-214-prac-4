#ifndef REALTRADER_H
#define REALTRADER_H

#include "TradeExecutor.h"
#include "Signal.h"

class RealTrader : public TradeExecutor
{
public:
    RealTrader();

    Signal execute(Signal decision) override;
};

#endif
