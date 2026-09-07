#ifndef TRADINGTECHNIQUE_H
#define TRADINGTECHNIQUE_H

#include <string>
#include <set>
#include "WorkerGroup.h"
#include "MarketObserver.h"

class StockMarket;

class TradingTechnique : public WorkerGroup, public MarketObserver {
private:
    double fundBalance;
    std::set<std::string> tickers;

public:
    TradingTechnique(double startingBalance);

    void onPriceUpdate(std::string ticker, double price) override;

    void registerTicker(std::string ticker);
    void subscribeAll(StockMarket* market);
    const std::set<std::string>& getTickers() const;

    void adjustBalance(double amount);
    double getFundBalance() const;
};

#endif
