#include "Worker.h"
#include "WorkerState.h"
#include "IdleState.h"
#include "FullTraversalIterator.h"
#include "SignalReadyIterator.h"
#include <algorithm>

Worker::Worker(std::string workerId, std::string ticker, std::size_t referenceWindow)
{
    this->workerId = workerId;
    this->state = new IdleState();
    this->referenceWindow = referenceWindow;
    this->lastPrice = 0.0;
    this->hasPending = false;
    this->signalExecuted = false;
    if (!ticker.empty())
    {
        watchlist.push_back(ticker);
    }
}

Worker::~Worker()
{
    delete state;
}

void Worker::setState(WorkerState *newState)
{
    if (newState == state)
    {
        return;
    }
    delete state;
    state = newState;
}

WorkerState *Worker::getState() const
{
    return state;
}

bool Worker::watches(const std::string &ticker) const
{
    return std::find(watchlist.begin(), watchlist.end(), ticker) != watchlist.end();
}

void Worker::addTicker(const std::string &ticker)
{
    if (!watches(ticker))
    {
        watchlist.push_back(ticker);
    }
}

double Worker::getReferenceAverage() const
{
    if (recentPrices.empty())
    {
        return 0.0;
    }
    double sum = 0.0;
    for (double p : recentPrices)
    {
        sum += p;
    }
    return sum / static_cast<double>(recentPrices.size());
}

void Worker::recordPrice(double price)
{
    lastPrice = price;
    recentPrices.push_back(price);
    while (recentPrices.size() > referenceWindow)
    {
        recentPrices.pop_front();
    }
}

void Worker::raiseSignal(SignalType type, double quantity)
{
    pendingSignal = Signal(watchlist.empty() ? "" : watchlist.front(), type, quantity);
    hasPending = true;
    signalExecuted = false;
}

void Worker::clearSignal()
{
    hasPending = false;
    signalExecuted = false;
    pendingSignal = Signal();
}

bool Worker::hasSignal() const
{
    return hasPending;
}

Signal Worker::getSignal() const
{
    return pendingSignal;
}

void Worker::onPriceUpdate(std::string ticker, double price)
{
    if (!watches(ticker))
    {
        return;
    }
    state->handleUpdate(this, price);
}

WorkItemIterator *Worker::createIterator(std::string mode)
{
    if (mode == "signal")
    {
        return new SignalReadyIterator(this);
    }
    return new FullTraversalIterator(this);
}

void Worker::flatten(std::vector<WorkItem *> &out)
{
    out.push_back(this);
}

bool Worker::isSignalReady() const
{
    return state != nullptr && state->isSignalReady();
}

std::string Worker::report() const
{
    std::string label = workerId + " [" + (state ? state->name() : "null") + "]";
    if (hasPending && pendingSignal.isActive())
    {
        label += " signal=" + pendingSignal.typeName();
    }
    else
    {
        label += " signal=none";
    }
    return label;
}

void Worker::addWatchTicker(const std::string &ticker)
{
    addTicker(ticker);
}

std::vector<std::string> Worker::getTickers() const
{
    return watchlist;
}

void Worker::consumeOwnSignal(std::vector<Signal> &out)
{
    if (hasPending && pendingSignal.isActive() && !signalExecuted)
    {
        out.push_back(pendingSignal);
        signalExecuted = true;
    }
}
