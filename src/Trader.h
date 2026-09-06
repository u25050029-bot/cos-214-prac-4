#ifndef TRADER_H
#define TRADER_H

#include <string>
#include <vector>
#include "MarketObserver.h"
#include "TradeExecutor.h"
#include "Signal.h"

class TradingTechnique;
class WorkItem;
class StockMarket;

class Trader : public MarketObserver
{
private:
    TradingTechnique *technique;
    TradeExecutor *chain;

    std::vector<Signal> gatherSignals(WorkItem *root);
    Signal combineForTicker(const std::vector<Signal> &signals);

public:
    Trader(TradingTechnique *technique, TradeExecutor *chain);

    void onPriceUpdate(std::string ticker, double price) override;

    void subscribeTo(StockMarket *market);

    void runCycle(WorkItem *root);
};

#endif
