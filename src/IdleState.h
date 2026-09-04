#ifndef IDLESTATE_H
#define IDLESTATE_H

class IdleState : WorkerState {


public:
	void handleUpdate(Worker context, double price);
};

#endif
