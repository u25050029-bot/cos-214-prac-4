#ifndef WORKITEM_H
#define WORKITEM_H

#include <string>
#include <vector>
#include "WorkItemIterator.h"
#include "Signal.h"

class WorkItem
{
public:
    virtual ~WorkItem() = default;

    virtual void onPriceUpdate(std::string ticker, double price) = 0;

    virtual void decide() = 0;

    virtual WorkItemIterator *createIterator(std::string mode) = 0;

    virtual double getBalanceContribution() = 0;

    virtual void consumeSignals(std::vector<Signal> &out) = 0;

    virtual void flatten(std::vector<WorkItem *> &out) = 0;

    virtual bool isSignalReady() const;

    virtual std::string report() const;

    virtual void addWatchTicker(const std::string &ticker);

    virtual std::vector<std::string> getTickers() const;
};

#endif
