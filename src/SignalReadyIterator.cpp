#include "SignalReadyIterator.h"
#include "WorkItem.h"

SignalReadyIterator::SignalReadyIterator(WorkItem *root) : position(0)
{
    if (root != nullptr)
    {
        std::vector<WorkItem *> all;
        root->flatten(all);
        for (WorkItem *item : all)
        {
            if (item->isSignalReady())
            {
                snapshot.push_back(item);
            }
        }
    }
}

bool SignalReadyIterator::hasNext()
{
    return position < snapshot.size();
}

WorkItem *SignalReadyIterator::next()
{
    if (!hasNext())
    {
        return nullptr;
    }
    return snapshot[position++];
}
