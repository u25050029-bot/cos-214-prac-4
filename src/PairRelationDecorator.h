#ifndef PAIRRELATIONDECORATOR_H
#define PAIRRELATIONDECORATOR_H

class PairRelationDecorator : WorkerDecorator {

public:
	double historicalSpread;

	void recomputeSpread();

	void onPriceUpdate();

	void decide();
};

#endif
