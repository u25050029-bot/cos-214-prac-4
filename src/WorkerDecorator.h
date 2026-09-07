#ifndef WORKERDECORATOR_H
#define WORKERDECORATOR_H

#include <string>
#include "WorkItem.h"

class WorkerDecorator : public WorkItem {
protected:
    WorkItem* wrapped;

public:
    WorkerDecorator(WorkItem* wrapped);
    ~WorkerDecorator() override;

    void onPriceUpdate(std::string ticker, double price) override;
    void decide() override;
    WorkItemIterator* createIterator(std::string mode) override;
    double getBalanceContribution() override;
    void consumeSignals(std::vector<Signal>& out) override;
    void flatten(std::vector<WorkItem*>& out) override;
    bool isSignalReady() const override;
    std::string report() const override;
    void addWatchTicker(const std::string& ticker) override;
};

#endif
