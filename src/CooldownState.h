#ifndef COOLDOWNSTATE_H
#define COOLDOWNSTATE_H

class CooldownState : WorkerState {


public:
	void handleUpdate(Worker context, double price);
};

#endif
