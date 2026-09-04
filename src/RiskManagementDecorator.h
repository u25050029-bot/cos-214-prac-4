#ifndef RISKMANAGEMENTDECORATOR_H
#define RISKMANAGEMENTDECORATOR_H

class RiskManagementDecorator : WorkerDecorator {

private:
	double maxTradePct;
	double maxTickerExposure;

public:
	void onPriceUpdate();

	void decide();
};

#endif
