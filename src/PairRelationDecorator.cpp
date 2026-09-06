#include "PairRelationDecorator.h"
#include <cmath>

PairRelationDecorator::PairRelationDecorator(WorkItem* wrapped, std::string tickerA,
                                             std::string tickerB, double historicalSpread)
    : WorkerDecorator(wrapped), tickerA(tickerA), tickerB(tickerB),
      historicalSpread(historicalSpread), priceA(0.0), priceB(0.0),
      haveA(false), haveB(false), pairSignalRaised(false) {}

void PairRelationDecorator::recomputeSpread() {
    if (!haveA || !haveB) {
        return;
    }
    double currentSpread = priceA - priceB;
    double drift = std::fabs(currentSpread - historicalSpread);
    if (historicalSpread != 0.0 && drift / std::fabs(historicalSpread) >= 0.05) {
        pairSignalRaised = true;
    }
}

void PairRelationDecorator::onPriceUpdate(std::string ticker, double price) {
    WorkerDecorator::onPriceUpdate(ticker, price);

    if (ticker == tickerA) {
        priceA = price;
        haveA = true;
    } else if (ticker == tickerB) {
        priceB = price;
        haveB = true;
    }
    recomputeSpread();
}

void PairRelationDecorator::collectSignals(std::vector<Signal>& out) {
    WorkerDecorator::collectSignals(out);
    if (pairSignalRaised) {
        SignalType type = (priceA - priceB > historicalSpread) ? SignalType::SELL : SignalType::BUY;
        out.push_back(Signal(tickerA, type, "now", 1.0));
    }
}

bool PairRelationDecorator::hasPairSignal() const {
    return pairSignalRaised;
}
