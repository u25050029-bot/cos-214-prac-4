#ifndef MONITORINGSTATE_H
#define MONITORINGSTATE_H

#include "WorkerState.h"

class MonitoringState : public WorkerState
{
public:
    void handleUpdate(Worker *context, double price) override;
    std::string name() const override;
};

#endif
