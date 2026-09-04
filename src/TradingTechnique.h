#ifndef TRADINGTECHNIQUE_H
#define TRADINGTECHNIQUE_H

class TradingTechnique : WorkerGroup, MarketObserver {

public:
	double fundBalance;

	void onPriceUpdate(String ticker, double price);

	void subscribeAll();

	void adjustBalance(double amount);
};

#endif
