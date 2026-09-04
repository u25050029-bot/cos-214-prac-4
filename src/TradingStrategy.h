#ifndef TRADINGSTRATEGY_H
#define TRADINGSTRATEGY_H

class TradingStrategy : WorkerGroup, MarketObserver {

public:
	double fundBalance;

	void onPriceUpdate(String ticker, double price);

	void subscribeAll();

	void adjustBalance(double amount);
};

#endif
