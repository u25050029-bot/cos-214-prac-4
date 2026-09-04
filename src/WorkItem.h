#ifndef WORKITEM_H
#define WORKITEM_H

#include <string>
#include "WorkItemIterator.h"

class WorkItem {


public:
	virtual ~WorkItem() = default;

	virtual void onPriceUpdate(std::string ticker, double price) = 0;

	virtual void decide() = 0;

	virtual WorkItemIterator* createIterator(std::string mode) = 0;

	virtual double getBalanceContribution() = 0;
};

#endif
