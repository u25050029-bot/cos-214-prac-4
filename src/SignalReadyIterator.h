#ifndef SIGNALREADYITERATOR_H
#define SIGNALREADYITERATOR_H

#include <vector>
#include "WorkItemIterator.h"

class WorkItem;

class SignalReadyIterator : public WorkItemIterator
{
private:
    std::vector<WorkItem *> snapshot;
    std::size_t position;

public:
    SignalReadyIterator(WorkItem *root);

    bool hasNext() override;
    WorkItem *next() override;
};

#endif
