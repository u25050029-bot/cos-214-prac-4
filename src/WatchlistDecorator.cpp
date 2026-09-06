#include "WatchlistDecorator.h"

WatchlistDecorator::WatchlistDecorator(WorkItem* wrapped, std::vector<std::string> extraTickers)
    : WorkerDecorator(wrapped), extraTickers(extraTickers) {

    for (std::size_t t = 0; t < extraTickers.size(); ++t) {
        wrapped->addWatchTicker(extraTickers[t]);
    }
}

void WatchlistDecorator::onPriceUpdate(std::string ticker, double price) {

    WorkerDecorator::onPriceUpdate(ticker, price);
}
