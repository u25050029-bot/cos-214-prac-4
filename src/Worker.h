#ifndef WORKER_H
#define WORKER_H

#include <string>
#include <vector>
#include <deque>
#include "WorkItem.h"
#include "Signal.h"

class WorkerState;

class Worker : public WorkItem
{
private:
    std::string workerId;
    std::vector<std::string> watchlist;
    WorkerState *state;
    std::size_t referenceWindow;
    std::deque<double> recentPrices;
    double lastPrice;
    Signal pendingSignal;
    bool hasPending;
    bool signalExecuted;

public:
    Worker(std::string workerId, std::string ticker, std::size_t referenceWindow = 5);
    ~Worker() override;

    void setState(WorkerState *newState);
    WorkerState *getState() const;

    bool watches(const std::string &ticker) const;
    void addTicker(const std::string &ticker);

    double getReferenceAverage() const;
    void recordPrice(double price);

    void raiseSignal(SignalType type, double quantity);
    void clearSignal();
    bool hasSignal() const;
    Signal getSignal() const;

    void onPriceUpdate(std::string ticker, double price) override;
    WorkItemIterator *createIterator(std::string mode) override;
    void consumeOwnSignal(std::vector<Signal> &out) override;
    void flatten(std::vector<WorkItem *> &out) override;
    bool isSignalReady() const override;
    std::string report() const override;
    void addWatchTicker(const std::string &ticker) override;
    std::vector<std::string> getTickers() const override;
};

#endif
