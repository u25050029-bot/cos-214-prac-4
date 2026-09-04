#include "TradingTechnique.h"

TradingTechnique::TradingTechnique(double startingBalance) : fundBalance(startingBalance) {}

void TradingTechnique::onPriceUpdate(std::string ticker, double price)
{
	WorkerGroup::onPriceUpdate(ticker, price);
}

void TradingTechnique::subscribeAll()
{
	// need the stock market implementation first
}

void TradingTechnique::adjustBalance(double amount)
{
	fundBalance += amount;
}
