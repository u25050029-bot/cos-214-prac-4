#ifndef MARKETOBSERVER_H
#define MARKETOBSERVER_H

class MarketObserver {


public:
	void onPriceUpdate(String ticker, double price);
};

#endif
