#ifndef SIGNALDETECTEDSTATE_H
#define SIGNALDETECTEDSTATE_H

#include "WorkerState.h"

class SignalDetectedState : public WorkerState
{
public:
    void handleUpdate(Worker *context, double price) override;
    std::string name() const override;
    bool isSignalReady() const override;
};

#endif
