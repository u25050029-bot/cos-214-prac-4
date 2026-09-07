#include "Trader.h"
#include "TradingTechnique.h"
#include "StockMarket.h"
#include "WorkItem.h"
#include "WorkItemIterator.h"
#include <map>
#include <set>
#include <vector>

Trader::Trader(TradingTechnique* technique, TradeExecutor* chain)
    : technique(technique), chain(chain) {}

void Trader::onPriceUpdate(std::string ticker, double price) {

    if (technique != nullptr) {
        technique->onPriceUpdate(ticker, price);
        runCycle(technique);
    }
}

void Trader::subscribeTo(StockMarket* market) {
    if (technique == nullptr || market == nullptr) {
        return;
    }
    const std::set<std::string>& tickers = technique->getTickers();
    for (std::set<std::string>::const_iterator it = tickers.begin();
         it != tickers.end(); ++it) {
        market->attach(this, *it);
    }
}

std::vector<Signal> Trader::gatherSignals(WorkItem* root) {
    std::vector<Signal> signals;
    if (root == nullptr) {
        return signals;
    }
    root->consumeSignals(signals);
    return signals;
}

Signal Trader::combineForTicker(const std::vector<Signal>& signals) {

    bool hasSell = false;
    bool hasBuy = false;
    double buyQty = 0.0;
    double sellQty = 0.0;
    std::string ticker = signals.front().getTicker();

    for (const Signal& s : signals) {
        if (s.getType() == SignalType::SELL) {
            hasSell = true;
            sellQty += s.getQuantity();
        } else if (s.getType() == SignalType::BUY) {
            hasBuy = true;
            buyQty += s.getQuantity();
        }
    }

    if (hasSell) {
        return Signal(ticker, SignalType::SELL, "now", sellQty > 0.0 ? sellQty : 1.0);
    }
    if (hasBuy) {
        return Signal(ticker, SignalType::BUY, "now", buyQty);
    }
    return Signal(ticker, SignalType::HOLD, "now", 0.0);
}

void Trader::runCycle(WorkItem* root) {
    std::vector<Signal> signals = gatherSignals(root);

    std::map<std::string, std::vector<Signal> > byTicker;
    for (const Signal& s : signals) {
        byTicker[s.getTicker()].push_back(s);
    }

    for (std::map<std::string, std::vector<Signal> >::iterator it = byTicker.begin();
         it != byTicker.end(); ++it) {
        Signal combined = combineForTicker(it->second);

        if (combined.getType() == SignalType::HOLD) {
            continue;
        }

        double notional = combined.getQuantity() * 100.0;
        if (combined.getType() == SignalType::BUY && notional > technique->getFundBalance()) {
            continue;
        }

        chain->execute(combined);
    }
}
