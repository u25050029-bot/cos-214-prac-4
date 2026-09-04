#ifndef WATCHLISTDECORATOR_H
#define WATCHLISTDECORATOR_H

class WatchlistDecorator : WorkerDecorator {

public:
	String extraTickers;

	void onPriceUpdate();

	void decide();
};

#endif
