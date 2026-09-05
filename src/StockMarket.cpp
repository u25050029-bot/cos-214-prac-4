#include "StockMarket.h"
#include <algorithm>

void StockMarket::attach(MarketObserver *observer, std::string ticker)
{
    observers[ticker].push_back(observer);
}

void StockMarket::detach(MarketObserver *observer, std::string ticker)
{
    std::map<std::string, std::vector<MarketObserver *>>::iterator it = observers.find(ticker);
    if (it == observers.end())
    {
        return;
    }
    std::vector<MarketObserver *> &subs = it->second;
    subs.erase(std::remove(subs.begin(), subs.end(), observer), subs.end());
}

void StockMarket::notify(std::string ticker, double price)
{
    std::vector<MarketObserver *> &subs = observers[ticker];
    for (std::size_t i = 0; i < subs.size(); ++i)
    {
        subs[i]->onPriceUpdate(ticker, price);
    }
}

void StockMarket::setStockPrice(std::string ticker, double price)
{
    prices[ticker] = price;
}

double StockMarket::getStockPrice(std::string ticker)
{
    std::map<std::string, double>::iterator it = prices.find(ticker);
    if (it == prices.end())
    {
        return 0.0;
    }
    return it->second;
}

void StockMarket::tick(std::string ticker, double price)
{
    prices[ticker] = price;
    notify(ticker, price);
}
