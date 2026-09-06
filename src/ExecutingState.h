#ifndef EXECUTINGSTATE_H
#define EXECUTINGSTATE_H

#include "WorkerState.h"

class ExecutingState : public WorkerState
{
private:
    int ticksWorked;
    int workDuration;

public:
    ExecutingState(int workDuration = 2);
    void handleUpdate(Worker *context, double price) override;
    std::string name() const override;
    bool isSignalReady() const override;
};

#endif
