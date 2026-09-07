#include "WorkerDecorator.h"

WorkerDecorator::WorkerDecorator(WorkItem *wrapped) : wrapped(wrapped) {}

WorkerDecorator::~WorkerDecorator()
{
    delete wrapped;
}

void WorkerDecorator::onPriceUpdate(std::string ticker, double price)
{
    wrapped->onPriceUpdate(ticker, price);
}

WorkItemIterator *WorkerDecorator::createIterator(std::string mode)
{
    return wrapped->createIterator(mode);
}

void WorkerDecorator::flatten(std::vector<WorkItem *> &out)
{
    wrapped->flatten(out);
}

bool WorkerDecorator::isSignalReady() const
{
    return wrapped->isSignalReady();
}

std::string WorkerDecorator::report() const
{
    return wrapped->report();
}

void WorkerDecorator::addWatchTicker(const std::string &ticker)
{
    wrapped->addWatchTicker(ticker);
}

std::vector<std::string> WorkerDecorator::getTickers() const
{
    return wrapped->getTickers();
}
