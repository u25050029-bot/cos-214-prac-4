#include "PairRelationDecorator.h"
#include <cmath>

PairRelationDecorator::PairRelationDecorator(WorkItem *wrapped, std::string tickerA,
                                             std::string tickerB, double historicalSpread)
    : WorkerDecorator(wrapped)
{
    this->tickerA = tickerA;
    this->tickerB = tickerB;
    this->historicalSpread = historicalSpread;
    this->priceA = 0.0;
    this->priceB = 0.0;
    this->haveA = false;
    this->haveB = false;
    this->pairSignalRaised = false;
}

void PairRelationDecorator::recomputeSpread()
{
    if (!haveA || !haveB)
    {
        return;
    }
    double currentSpread = priceA - priceB;
    double drift = std::fabs(currentSpread - historicalSpread);
    if (historicalSpread != 0.0 && drift / std::fabs(historicalSpread) >= 0.05)
    {
        pairSignalRaised = true;
    }
}

void PairRelationDecorator::onPriceUpdate(std::string ticker, double price)
{
    WorkerDecorator::onPriceUpdate(ticker, price);

    if (ticker == tickerA)
    {
        priceA = price;
        haveA = true;
    }
    else if (ticker == tickerB)
    {
        priceB = price;
        haveB = true;
    }
    recomputeSpread();
}

bool PairRelationDecorator::hasPairSignal() const
{
    return pairSignalRaised;
}

void PairRelationDecorator::consumeSignals(std::vector<Signal> &out)
{
    WorkerDecorator::consumeSignals(out);
    if (pairSignalRaised)
    {
        SignalType type = (priceA - priceB > historicalSpread) ? SignalType::SELL : SignalType::BUY;
        out.push_back(Signal(tickerA, type, "now", 1.0));
        pairSignalRaised = false;
    }
}
