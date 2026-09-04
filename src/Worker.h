#ifndef WORKER_H
#define WORKER_H

class Worker : WorkItem {

private:
	String watchlist;
	WorkerState state;

public:
	void setState(WorkerState newState);

	void getReferenceAverage();
};

#endif
