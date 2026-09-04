#include "WorkerGroup.h"
#include <algorithm>

WorkerGroup::~WorkerGroup()
{
	for (WorkItem* child : children) {
		delete child;
	}
}

void WorkerGroup::add(WorkItem* item)
{
	children.push_back(item);
}

void WorkerGroup::remove(WorkItem* item)
{
	std::vector<WorkItem*>::iterator it = std::find(children.begin(), children.end(), item);
	if (it != children.end()) {
		children.erase(it);
	}
}

void WorkerGroup::onPriceUpdate(std::string ticker, double price)
{
	for (WorkItem* child : children) {
		child->onPriceUpdate(ticker, price);
	}
}

void WorkerGroup::decide()
{
	for (WorkItem* child : children) {
		child->decide();
	}
}

WorkItemIterator* WorkerGroup::createIterator(std::string mode)
{
	return nullptr; // TODO: return FullTraversalIterator or SignalReadyIterator once Iterator slice is ready
}