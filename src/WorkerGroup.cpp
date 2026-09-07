#include "WorkerGroup.h"
#include "FullTraversalIterator.h"
#include "SignalReadyIterator.h"
#include <algorithm>

WorkerGroup::~WorkerGroup() {
    for (WorkItem* child : children) {
        delete child;
    }
}

void WorkerGroup::add(WorkItem* item) {
    children.push_back(item);
}

void WorkerGroup::remove(WorkItem* item) {
    std::vector<WorkItem*>::iterator it = std::find(children.begin(), children.end(), item);
    if (it != children.end()) {
        children.erase(it);
    }
}

void WorkerGroup::onPriceUpdate(std::string ticker, double price) {
    for (WorkItem* child : children) {
        child->onPriceUpdate(ticker, price);
    }
}

void WorkerGroup::decide() {
    for (WorkItem* child : children) {
        child->decide();
    }
}

WorkItemIterator* WorkerGroup::createIterator(std::string mode) {
    if (mode == "signal") {
        return new SignalReadyIterator(this);
    }
    return new FullTraversalIterator(this);
}

double WorkerGroup::getBalanceContribution() {
    double total = 0.0;
    for (WorkItem* child : children) {
        total += child->getBalanceContribution();
    }
    return total;
}

void WorkerGroup::collectSignals(std::vector<Signal>& out) {
    for (WorkItem* child : children) {
        child->collectSignals(out);
    }
}

void WorkerGroup::flatten(std::vector<WorkItem*>& out) {
    for (WorkItem* child : children) {
        child->flatten(out);
    }
}

std::string WorkerGroup::report() const {
    return "group(" + std::to_string(children.size()) + ")";
}

void WorkerGroup::addWatchTicker(const std::string& ticker) {
    for (WorkItem* child : children) {
        child->addWatchTicker(ticker);
    }
}
