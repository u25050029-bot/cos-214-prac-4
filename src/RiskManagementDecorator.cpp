#include "RiskManagementDecorator.h"
#include <iostream>

RiskManagementDecorator::RiskManagementDecorator(TradeExecutor *inner, double maxTradePct,
                                                 double maxTickerExposure)
{
    this->inner = inner;
    this->maxTradePct = maxTradePct;
    this->maxTickerExposure = maxTickerExposure;
}

RiskManagementDecorator::~RiskManagementDecorator()
{
    delete inner;
}

Signal RiskManagementDecorator::execute(Signal decision)
{
    if (decision.getQuantity() > maxTickerExposure)
    {
        std::cout << "[Risk] Shrinking " << decision.getTicker() << " from "
                  << decision.getQuantity() << " to " << maxTickerExposure << std::endl;
        decision.setQuantity(maxTickerExposure);
    }

    if (decision.getQuantity() <= 0.0)
    {
        std::cout << "[Risk] Downgrading " << decision.getTicker() << " to HOLD" << std::endl;
        decision.setType(SignalType::HOLD);
    }

    return inner->execute(decision);
}
