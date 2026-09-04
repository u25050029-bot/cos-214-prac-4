#ifndef STOCKMARKET_H
#define STOCKMARKET_H

class StockMarket {


public:
	void getCurrentStock(String ticker);

	double getStockPrice(String ticker);

	void subscribe(MarketObserver observer, String ticker);
};

#endif
