#ifndef RISKMANAGEMENTDECORATOR_H
#define RISKMANAGEMENTDECORATOR_H

#include "TradeExecutor.h"
#include "Signal.h"

class RiskManagementDecorator : public TradeExecutor
{
private:
    TradeExecutor *inner;
    double maxTradePct;
    double maxTickerExposure;

public:
    RiskManagementDecorator(TradeExecutor *inner, double maxTradePct, double maxTickerExposure);
    ~RiskManagementDecorator() override;

    Signal execute(Signal decision) override;
};

#endif
