#ifndef TRADEEXECUTOR_H
#define TRADEEXECUTOR_H

#include "Signal.h"

class TradeExecutor
{
public:
    virtual ~TradeExecutor() = default;

    virtual Signal execute(Signal decision) = 0;
};

#endif
