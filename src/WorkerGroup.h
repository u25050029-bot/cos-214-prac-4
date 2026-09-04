#ifndef WORKERGROUP_H
#define WORKERGROUP_H

class WorkerGroup : WorkItem {

public:
	WorkItem children;

	void add(WorkItem item);

	void remove(WorkItem item);
};

#endif
