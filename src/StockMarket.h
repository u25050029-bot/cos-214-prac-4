#ifndef STOCKMARKET_H
#define STOCKMARKET_H
#include "MarketObserver.h"
class StockMarket {
private:
std::vetor<MarketObserver*> children;

public:
	void getCurrentStock(String ticker);
	double getStockPrice(String ticker);
	void subscribe(MarketObserver observer, String ticker);
	void subscribe(MarketObserver observer, String ticker);
	void notify(std::string sticker, double price);
};

#endif
