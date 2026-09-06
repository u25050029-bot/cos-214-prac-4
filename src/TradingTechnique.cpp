#include "TradingTechnique.h"
#include "StockMarket.h"

TradingTechnique::TradingTechnique(double startingBalance)
{
    this->fundBalance = startingBalance;
}

void TradingTechnique::onPriceUpdate(std::string ticker, double price)
{
    WorkerGroup::onPriceUpdate(ticker, price);
}

void TradingTechnique::registerTicker(std::string ticker)
{
    tickers.insert(ticker);
}

void TradingTechnique::subscribeAll(StockMarket *market)
{
    for (std::set<std::string>::iterator it = tickers.begin(); it != tickers.end(); ++it)
    {
        market->attach(this, *it);
    }
}

void TradingTechnique::adjustBalance(double amount)
{
    fundBalance += amount;
}

double TradingTechnique::getFundBalance() const
{
    return fundBalance;
}
