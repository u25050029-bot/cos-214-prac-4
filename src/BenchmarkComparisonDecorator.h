#ifndef BENCHMARKCOMPARISONDECORATOR_H
#define BENCHMARKCOMPARISONDECORATOR_H

class BenchmarkComparisonDecorator : WorkerDecorator {

public:
	String indexTicker;

	void compareGrowth();

	void onPriceUpdate();

	void decide();
};

#endif
