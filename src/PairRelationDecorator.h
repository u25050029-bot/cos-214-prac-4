#ifndef PAIRRELATIONDECORATOR_H
#define PAIRRELATIONDECORATOR_H

#include <string>
#include "WorkerDecorator.h"

class PairRelationDecorator : public WorkerDecorator
{
private:
    std::string tickerA;
    std::string tickerB;
    double historicalSpread;
    double priceA;
    double priceB;
    bool haveA;
    bool haveB;
    bool pairSignalRaised;

    void recomputeSpread();

public:
    PairRelationDecorator(WorkItem *wrapped, std::string tickerA, std::string tickerB,
                          double historicalSpread);

    void onPriceUpdate(std::string ticker, double price) override;
    void consumeOwnSignal(std::vector<Signal> &out) override;
    void flatten(std::vector<WorkItem *> &out) override;
    bool isSignalReady() const override;
};

#endif
