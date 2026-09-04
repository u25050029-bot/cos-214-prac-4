#ifndef MONITORINGSTATE_H
#define MONITORINGSTATE_H

class MonitoringState : WorkerState {


public:
	void handleUpdate(Worker context, double price);
};

#endif
