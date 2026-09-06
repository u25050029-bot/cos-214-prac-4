#include "ComplianceReviewDecorator.h"
#include "TradingTechnique.h"
#include <algorithm>
#include <iostream>

ComplianceReviewDecorator::ComplianceReviewDecorator(TradeExecutor *inner,
                                                     TradingTechnique *technique,
                                                     std::vector<std::string> restrictedList)
{
    this->inner = inner;
    this->technique = technique;
    this->restrictedList = restrictedList;
}

ComplianceReviewDecorator::~ComplianceReviewDecorator()
{
    delete inner;
}

bool ComplianceReviewDecorator::isRestricted(const std::string &ticker) const
{
    return std::find(restrictedList.begin(), restrictedList.end(), ticker) != restrictedList.end();
}

void ComplianceReviewDecorator::logDecision(const Signal &decision)
{
    std::cout << "[Compliance] Logged " << decision.typeName() << " " << decision.getTicker()
              << " qty=" << decision.getQuantity() << std::endl;
}

Signal ComplianceReviewDecorator::execute(Signal decision)
{

    Signal result = inner->execute(decision);

    if (isRestricted(result.getTicker()) && result.getType() != SignalType::HOLD)
    {
        std::cout << "[Compliance] " << result.getTicker()
                  << " is restricted - overriding to HOLD" << std::endl;
        result.setType(SignalType::HOLD);
    }

    logDecision(result);
    if (technique != nullptr && result.getType() != SignalType::HOLD)
    {
        double notional = result.getQuantity() * 100.0;
        technique->adjustBalance(result.getType() == SignalType::BUY ? -notional : notional);
    }
    return result;
}

void ComplianceReviewDecorator::restrict(const std::string &ticker)
{
    restrictedList.push_back(ticker);
}
