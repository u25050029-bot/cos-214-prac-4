#ifndef FULLTRAVERSALITERATOR_H
#define FULLTRAVERSALITERATOR_H

#include <vector>
#include "WorkItemIterator.h"

class WorkItem;

class FullTraversalIterator : public WorkItemIterator
{
private:
    std::vector<WorkItem *> snapshot;
    std::size_t position;

public:
    FullTraversalIterator(WorkItem *root);

    bool hasNext() override;
    WorkItem *next() override;
};

#endif
