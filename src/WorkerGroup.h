#ifndef WORKERGROUP_H
#define WORKERGROUP_H

#include <vector>
#include "WorkItem.h"

class WorkerGroup : public WorkItem {

private:
	std::vector<WorkItem*> children;

public:
	~WorkerGroup();

	void add(WorkItem* item);

	void remove(WorkItem* item);

	void onPriceUpdate(std::string ticker, double price) override;

	void decide() override;

	double getBalanceContribution() override;

	WorkItemIterator* createIterator(std::string mode);
};

#endif
