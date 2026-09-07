#include "BenchmarkComparisonDecorator.h"

BenchmarkComparisonDecorator::BenchmarkComparisonDecorator(WorkItem* wrapped,
                                                           std::string trackedTicker,
                                                           std::string indexTicker,
                                                           double trackedBaseline,
                                                           double indexBaseline)
    : WorkerDecorator(wrapped), trackedTicker(trackedTicker), indexTicker(indexTicker),
      indexBaseline(indexBaseline), trackedBaseline(trackedBaseline),
      indexPrice(0.0), trackedPrice(0.0), haveIndex(false), haveTracked(false),
      benchmarkSignalRaised(false), benchmarkSignalType(SignalType::HOLD) {}

void BenchmarkComparisonDecorator::compareGrowth() {
    if (!haveIndex || !haveTracked) {
        return;
    }
    if (trackedBaseline <= 0.0 || indexBaseline <= 0.0) {
        return;
    }

    double trackedGrowth = (trackedPrice - trackedBaseline) / trackedBaseline;
    double indexGrowth = (indexPrice - indexBaseline) / indexBaseline;
    double relative = trackedGrowth - indexGrowth;

    if (relative >= 0.02) {
        benchmarkSignalRaised = true;
        benchmarkSignalType = SignalType::BUY;
    } else if (relative <= -0.02) {
        benchmarkSignalRaised = true;
        benchmarkSignalType = SignalType::SELL;
    }
}

void BenchmarkComparisonDecorator::onPriceUpdate(std::string ticker, double price) {

    wrapped->onPriceUpdate(ticker, price);

    if (ticker == indexTicker) {
        indexPrice = price;
        haveIndex = true;
    } else if (ticker == trackedTicker) {
        trackedPrice = price;
        haveTracked = true;
    }
    compareGrowth();
}

bool BenchmarkComparisonDecorator::hasBenchmarkSignal() const {
    return benchmarkSignalRaised;
}

void BenchmarkComparisonDecorator::consumeSignals(std::vector<Signal>& out) {
    WorkerDecorator::consumeSignals(out);
    if (benchmarkSignalRaised) {
        out.push_back(Signal(trackedTicker, benchmarkSignalType, "now", 1.0));
        benchmarkSignalRaised = false;
    }
}
