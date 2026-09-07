#ifndef BENCHMARKCOMPARISONDECORATOR_H
#define BENCHMARKCOMPARISONDECORATOR_H

#include <string>
#include <vector>
#include "WorkerDecorator.h"
#include "Signal.h"

class BenchmarkComparisonDecorator : public WorkerDecorator {
private:
    std::string trackedTicker;
    std::string indexTicker;
    double indexBaseline;
    double trackedBaseline;
    double indexPrice;
    double trackedPrice;
    bool haveIndex;
    bool haveTracked;
    bool benchmarkSignalRaised;
    SignalType benchmarkSignalType;

    void compareGrowth();

public:
    BenchmarkComparisonDecorator(WorkItem* wrapped, std::string trackedTicker,
                                 std::string indexTicker, double trackedBaseline,
                                 double indexBaseline);

    void onPriceUpdate(std::string ticker, double price) override;
    void consumeSignals(std::vector<Signal>& out) override;

    bool hasBenchmarkSignal() const;
};

#endif
