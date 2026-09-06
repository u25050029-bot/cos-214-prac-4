#ifndef WORKERGROUP_H
#define WORKERGROUP_H

#include <vector>
#include <string>
#include "WorkItem.h"

class WorkerGroup : public WorkItem
{
protected:
    std::vector<WorkItem *> children;

public:
    ~WorkerGroup() override;

    void add(WorkItem *item);
    void remove(WorkItem *item);

    void onPriceUpdate(std::string ticker, double price) override;
    void decide() override;
    WorkItemIterator *createIterator(std::string mode) override;
    double getBalanceContribution() override;
    void consumeSignals(std::vector<Signal> &out) override;
    void flatten(std::vector<WorkItem *> &out) override;
    std::string report() const override;
    void addWatchTicker(const std::string &ticker) override;
    std::vector<std::string> getTickers() const override;
};

#endif
