#ifndef WORKERGROUP_H
#define WORKERGROUP_H

class WorkerGroup : WorkItem {

private:
	WorkItem children;
	TradingTechnique tradingTechnique;

public:
	void add(WorkItem item);

	void remove(WorkItem item);
};

#endif
