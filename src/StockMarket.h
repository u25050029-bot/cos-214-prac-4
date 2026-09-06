#ifndef STOCKMARKET_H
#define STOCKMARKET_H

#include <string>
#include <map>
#include <vector>
#include "MarketSubject.h"
#include "MarketObserver.h"

class StockMarket : public MarketSubject {
private:
    std::map<std::string, std::vector<MarketObserver*> > observers;
    std::map<std::string, double> prices;

public:
    void attach(MarketObserver* observer, std::string ticker) override;
    void detach(MarketObserver* observer, std::string ticker) override;
    void notify(std::string ticker, double price) override;

    void setStockPrice(std::string ticker, double price);
    double getStockPrice(std::string ticker);

    void tick(std::string ticker, double price);
};

#endif
