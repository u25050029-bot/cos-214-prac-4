#include "Trader.h"
#include <iostream>
#include "TradingTechnique.h"
#include "StockMarket.h"
#include "WorkItem.h"
#include "WorkItemIterator.h"
#include <map>
#include <set>
#include <vector>

Trader::Trader(TradingTechnique *technique, TradeExecutor *chain)
{
    this->technique = technique;
    this->chain = chain;
}

void Trader::onPriceUpdate(std::string ticker, double price)
{

    if (technique != nullptr)
    {
        technique->onPriceUpdate(ticker, price);
        runCycle(technique);
    }
}

void Trader::subscribeTo(StockMarket *market)
{
    if (technique == nullptr || market == nullptr)
    {
        return;
    }

    std::vector<std::string> allTickers;
    WorkItemIterator *it = technique->createIterator("full");
    while (it->hasNext())
    {
        WorkItem *item = it->next();
        std::vector<std::string> itemTickers = item->getTickers();
        for (std::size_t i = 0; i < itemTickers.size(); ++i)
        {
            allTickers.push_back(itemTickers[i]);
        }
    }
    delete it;

    std::cout << "All tickers (with duplicates):";
    for (std::size_t i = 0; i < allTickers.size(); ++i)
    {
        std::cout << " " << allTickers[i];
    }
    std::cout << std::endl;

    std::vector<std::string> uniqueTickers;
    for (std::size_t i = 0; i < allTickers.size(); ++i)
    {
        bool alreadySeen = false;
        for (std::size_t j = 0; j < uniqueTickers.size(); ++j)
        {
            if (uniqueTickers[j] == allTickers[i])
            {
                alreadySeen = true;
                break;
            }
        }
        if (!alreadySeen)
        {
            uniqueTickers.push_back(allTickers[i]);
        }
    }

    std::cout << "Unique tickers:";
    for (std::size_t i = 0; i < uniqueTickers.size(); ++i)
    {
        std::cout << " " << uniqueTickers[i];
    }
    std::cout << std::endl;

    for (std::size_t i = 0; i < uniqueTickers.size(); ++i)
    {
        market->attach(this, uniqueTickers[i]);
    }
}

std::vector<Signal> Trader::gatherSignals(WorkItem *root)
{
    std::vector<Signal> signals;
    if (root == nullptr)
    {
        return signals;
    }
    root->consumeSignals(signals);
    return signals;
}

Signal Trader::combineForTicker(const std::vector<Signal> &signals)
{

    bool hasSell = false;
    bool hasBuy = false;
    double buyQty = 0.0;
    double sellQty = 0.0;
    std::string ticker = signals.front().getTicker();

    for (const Signal &s : signals)
    {
        if (s.getType() == SignalType::SELL)
        {
            hasSell = true;
            sellQty += s.getQuantity();
        }
        else if (s.getType() == SignalType::BUY)
        {
            hasBuy = true;
            buyQty += s.getQuantity();
        }
    }

    if (hasSell)
    {
        return Signal(ticker, SignalType::SELL, "now", sellQty > 0.0 ? sellQty : 1.0);
    }
    if (hasBuy)
    {
        return Signal(ticker, SignalType::BUY, "now", buyQty);
    }
    return Signal(ticker, SignalType::HOLD, "now", 0.0);
}

void Trader::runCycle(WorkItem *root)
{
    std::vector<Signal> signals = gatherSignals(root);

    std::map<std::string, std::vector<Signal>> byTicker;
    for (const Signal &s : signals)
    {
        byTicker[s.getTicker()].push_back(s);
    }

    for (std::map<std::string, std::vector<Signal>>::iterator it = byTicker.begin();
         it != byTicker.end(); ++it)
    {
        Signal combined = combineForTicker(it->second);

        if (combined.getType() == SignalType::HOLD)
        {
            continue;
        }

        double notional = combined.getQuantity() * 100.0;
        if (combined.getType() == SignalType::BUY && notional > technique->getFundBalance())
        {
            continue;
        }

        chain->execute(combined);
    }
}
