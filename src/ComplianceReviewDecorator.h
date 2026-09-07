#ifndef COMPLIANCEREVIEWDECORATOR_H
#define COMPLIANCEREVIEWDECORATOR_H

#include <string>
#include <vector>
#include "TradeExecutor.h"
#include "Signal.h"

class TradingTechnique;

class ComplianceReviewDecorator : public TradeExecutor
{
private:
    TradeExecutor *inner;
    TradingTechnique *technique;
    std::vector<std::string> restrictedList;

    void logDecision(const Signal &decision);
    bool isRestricted(const std::string &ticker) const;

public:
    ComplianceReviewDecorator(TradeExecutor *inner, TradingTechnique *technique,
                              std::vector<std::string> restrictedList);
    ~ComplianceReviewDecorator() override;

    void restrict(const std::string &ticker);

    Signal execute(Signal decision) override;
};

#endif
