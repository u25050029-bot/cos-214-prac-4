#ifndef TRADINGTECHNIQUE_H
#define TRADINGTECHNIQUE_H

#include <string>
#include "WorkerGroup.h"
#include "MarketObserver.h"


class TradingTechnique : public WorkerGroup, public MarketObserver {

private:
	double fundBalance;
public:
	TradingTechnique(double startingBalance);

	void onPriceUpdate(std::string ticker, double price) override;

	void subscribeAll();

	void adjustBalance(double amount);
};

#endif
