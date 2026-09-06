#include "FullTraversalIterator.h"
#include "WorkItem.h"

FullTraversalIterator::FullTraversalIterator(WorkItem *root) : position(0)
{
    if (root != nullptr)
    {
        root->flatten(snapshot);
    }
}

bool FullTraversalIterator::hasNext()
{
    return position < snapshot.size();
}

WorkItem *FullTraversalIterator::next()
{
    if (!hasNext())
    {
        return nullptr;
    }
    return snapshot[position++];
}
