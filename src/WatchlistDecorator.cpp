#include "WatchlistDecorator.h"
#include "Worker.h"
#include <vector>

WatchlistDecorator::WatchlistDecorator(WorkItem* wrapped, std::vector<std::string> extraTickers)
    : WorkerDecorator(wrapped), extraTickers(extraTickers) {

    std::vector<WorkItem*> leaves;
    wrapped->flatten(leaves);
    for (std::size_t i = 0; i < leaves.size(); ++i) {
        Worker* w = dynamic_cast<Worker*>(leaves[i]);
        if (w != nullptr) {
            for (std::size_t t = 0; t < extraTickers.size(); ++t) {
                w->addTicker(extraTickers[t]);
            }
        }
    }
}

void WatchlistDecorator::onPriceUpdate(std::string ticker, double price) {

    WorkerDecorator::onPriceUpdate(ticker, price);
}
