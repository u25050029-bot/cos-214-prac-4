#ifndef IDLESTATE_H
#define IDLESTATE_H

#include "WorkerState.h"

class IdleState : public WorkerState
{
public:
    void handleUpdate(Worker *context, double price) override;
    std::string name() const override;
};

#endif
