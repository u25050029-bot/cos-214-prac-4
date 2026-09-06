#ifndef WORKERSTATE_H
#define WORKERSTATE_H

#include <string>

class Worker;

class WorkerState
{
public:
    virtual ~WorkerState() = default;

    virtual void handleUpdate(Worker *context, double price) = 0;

    virtual std::string name() const = 0;

    virtual bool isSignalReady() const;
};

#endif
