#ifndef COOLDOWNSTATE_H
#define COOLDOWNSTATE_H

#include "WorkerState.h"

class CooldownState : public WorkerState
{
private:
    int ticksRemaining;

public:
    CooldownState(int ticks = 3);
    void handleUpdate(Worker *context, double price) override;
    std::string name() const override;
};

#endif
