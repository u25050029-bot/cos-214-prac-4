#ifndef WORKITEM_H
#define WORKITEM_H

class WorkItem {


public:
	void onPriceUpdate(String ticker, double price);

	void decide();

	WorkItemIterator createIterator(IteratorMode mode);

	void getBalanceContribution();
};

#endif
