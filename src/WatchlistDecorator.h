#ifndef WATCHLISTDECORATOR_H
#define WATCHLISTDECORATOR_H

#include <string>
#include <vector>
#include "WorkerDecorator.h"

class WatchlistDecorator : public WorkerDecorator {
private:
    std::vector<std::string> extraTickers;

public:
    WatchlistDecorator(WorkItem* wrapped, std::vector<std::string> extraTickers);

    void onPriceUpdate(std::string ticker, double price) override;
};

#endif
