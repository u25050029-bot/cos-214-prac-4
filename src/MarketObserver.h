#ifndef MARKETOBSERVER_H
#define MARKETOBSERVER_H

class MarketObserver {


public:
	virtual ~MarketObserver() = default;
	void onPriceUpdate(String ticker, double price)=0;
};

#endif
