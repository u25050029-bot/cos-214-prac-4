#ifndef SIGNALDETECTEDSTATE_H
#define SIGNALDETECTEDSTATE_H

class SignalDetectedState : WorkerState {


public:
	void handleUpdate(Worker context, double price);
};

#endif
