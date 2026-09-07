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

void TradingTechnique::adjustBalance(double amount)
{
    fundBalance += amount;
}

double TradingTechnique::getFundBalance() const
{
    return fundBalance;
}
