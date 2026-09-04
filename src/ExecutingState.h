#ifndef EXECUTINGSTATE_H
#define EXECUTINGSTATE_H

class ExecutingState : WorkerState {


public:
	void handleUpdate(Worker context, double price);
};

#endif
