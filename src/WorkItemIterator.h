#ifndef WORKITEMITERATOR_H
#define WORKITEMITERATOR_H

class WorkItem;

class WorkItemIterator
{
public:
    virtual ~WorkItemIterator() = default;

    virtual bool hasNext() = 0;
    virtual WorkItem *next() = 0;
};

#endif
