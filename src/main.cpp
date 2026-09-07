#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>

#include "StockMarket.h"
#include "TradingTechnique.h"
#include "Division.h"
#include "Worker.h"
#include "PairRelationDecorator.h"
#include "WatchlistDecorator.h"
#include "BenchmarkComparisonDecorator.h"
#include "TradeExecutor.h"
#include "RealTrader.h"
#include "RiskManagementDecorator.h"
#include "ComplianceReviewDecorator.h"
#include "Trader.h"

struct Instrument
{
    std::string ticker;
    double baseline;
};

static const Instrument NVDA = {"NVDA", 120.0}; // nvidia
static const Instrument AMD = {"AMD", 160.0};   // amd
static const Instrument KO = {"KO", 60.0};      // coke
static const Instrument PEP = {"PEP", 55.0};    // pepsi
static const Instrument SPX = {"SPX", 5000.0};  // spx

enum EventKind
{
    CALM = 0,  // tiny move  -> keeps Idle / drifts Monitoring back to Idle
    STIR,      // ~1.2% move -> Idle -> Monitoring
    RALLY,     // ~3% up     -> Monitoring -> SignalDetected (BUY)
    SELLOFF,   // ~3% down   -> Monitoring -> SignalDetected (SELL)
    HOLD_HIGH, // stays high -> SignalDetected -> Executing -> Cooldown -> Idle
    HOLD_LOW,  // stays low  -> same, on the SELL side
    EVENT_KIND_COUNT
};

static const char *eventName(EventKind kind)
{
    switch (kind)
    {
    case CALM:
        return "CALM";
    case STIR:
        return "STIR";
    case RALLY:
        return "RALLY";
    case SELLOFF:
        return "SELLOFF";
    case HOLD_HIGH:
        return "HOLD_HIGH";
    case HOLD_LOW:
        return "HOLD_LOW";
    default:
        return "?";
    }
}

// Convert an event kind into a multiplier on an instrument's baseline.
static double eventMultiplier(EventKind kind)
{
    switch (kind)
    {
    case CALM:
        return 1.001; // +0.1%
    case STIR:
        return 1.012; // +1.2%
    case RALLY:
        return 1.030; // +3.0%
    case SELLOFF:
        return 0.970; // -3.0%
    case HOLD_HIGH:
        return 1.035; // hold above the reference
    case HOLD_LOW:
        return 0.965; // hold below the reference
    default:
        return 1.000;
    }
}

// just to simplify data
struct MarketEvent
{
    Instrument instrument;
    EventKind kind;
};

int main()
{
    std::cout << "Stock Market Simulation" << std::endl;

    //
    StockMarket exchange;

    // build structure

    // root of tree
    TradingTechnique *momentumFund = new TradingTechnique(1000000.0); // startingBalance

    // Creating a division for tech stocks
    Division *techDesk = new Division();

    Worker *nvidiaLead = new Worker("nvidia-lead", NVDA.ticker, 8); // workerId, ticker, referenceWindow

    Worker *nvidiaMomentum = new Worker("nvidia-momentum", NVDA.ticker, 8); // workerId, ticker, referenceWindow

    Worker *amdAnalyst = new Worker("amd-analyst", AMD.ticker, 8); // workerId, ticker, referenceWindow

    // A benchmark-decorated view of the AMD analyst, compared against the index.
    BenchmarkComparisonDecorator *amdVsIndex = new BenchmarkComparisonDecorator(amdAnalyst, AMD.ticker, SPX.ticker, AMD.baseline, SPX.baseline); // wrapped, trackedTicker, indexTicker, trackedBaseline, indexBaseline

    techDesk->add(nvidiaLead);
    techDesk->add(nvidiaMomentum);
    techDesk->add(amdVsIndex);

    // Division for bevrages
    Division *beveragesDesk = new Division();

    Worker *cokeAnalyst = new Worker("coke-analyst", KO.ticker, 8); // workerId, ticker, referenceWindow

    Worker *pepsiAnalyst = new Worker("pepsi-analyst", PEP.ticker, 8); // workerId, ticker, referenceWindow

    beveragesDesk->add(cokeAnalyst);
    beveragesDesk->add(pepsiAnalyst);

    // Coke vs pepsi 
    PairRelationDecorator *beveragesPair = new PairRelationDecorator(beveragesDesk, KO.ticker, PEP.ticker, KO.baseline - PEP.baseline); // wrapped, tickerA, tickerB, historicalSpread

    Worker *crossAnalyst = new Worker("cross-analyst", NVDA.ticker, 8); // workerId, ticker, referenceWindow
    std::vector<std::string> extraTickers;
    extraTickers.push_back(AMD.ticker);
    WatchlistDecorator *crossCovered = new WatchlistDecorator(crossAnalyst, extraTickers); // wrapped, extraTickers

    // Building the tree
    momentumFund->add(techDesk);
    momentumFund->add(beveragesPair);
    momentumFund->add(crossCovered);

    // Decision pipeline
    TradeExecutor *executor = new RealTrader();

    TradeExecutor *riskLayer = new RiskManagementDecorator(executor, 0.5, 100.0); // inner, maxTradePct, maxTickerExposure

    std::vector<std::string> restrictedTickers;
    ComplianceReviewDecorator *complianceLayer = new ComplianceReviewDecorator(riskLayer, momentumFund, restrictedTickers); // inner, technique, restrictedList
    // Observer
    Trader *deskTrader = new Trader(momentumFund, complianceLayer); // technique, chain  // will own the root
    deskTrader->subscribeAll(&exchange);

    // Creating data
    const int TOTAL_TICKS = 200;
    int eventWeight[EVENT_KIND_COUNT];
    eventWeight[CALM] = 40;
    eventWeight[STIR] = 25;
    eventWeight[RALLY] = 12;
    eventWeight[SELLOFF] = 8;
    eventWeight[HOLD_HIGH] = 10;
    eventWeight[HOLD_LOW] = 5;
    int weightTotal = 0;
    for (int k = 0; k < EVENT_KIND_COUNT; ++k)
        weightTotal += eventWeight[k];

    std::vector<Instrument> tradeUniverse;
    tradeUniverse.push_back(NVDA);
    tradeUniverse.push_back(AMD);
    tradeUniverse.push_back(KO);
    tradeUniverse.push_back(PEP);

    std::vector<EventKind> eventKinds;
    eventKinds.reserve(TOTAL_TICKS);
    for (int k = 0; k < EVENT_KIND_COUNT; ++k)
    {
        int count = (eventWeight[k] * TOTAL_TICKS) / weightTotal;
        for (int i = 0; i < count; ++i)
            eventKinds.push_back((EventKind)k);
    }
    for (int k = 0; k < EVENT_KIND_COUNT; ++k) // guarantee full coverage
    {
        bool present = false;
        for (std::size_t i = 0; i < eventKinds.size(); ++i)
            if (eventKinds[i] == (EventKind)k)
            {
                present = true;
                break;
            }
        if (!present)
            eventKinds.push_back((EventKind)k);
    }
    while ((int)eventKinds.size() < TOTAL_TICKS)
        eventKinds.push_back(CALM);
    if ((int)eventKinds.size() > TOTAL_TICKS)
        eventKinds.resize(TOTAL_TICKS);

    // shuffle events
    std::srand(2026); // setting seed
    for (int i = TOTAL_TICKS - 1; i > 0; --i)
    {
        int j = std::rand() % (i + 1);
        EventKind tmp = eventKinds[i];
        eventKinds[i] = eventKinds[j];
        eventKinds[j] = tmp;
    }

    // process data into instrument, update
    std::vector<MarketEvent> schedule;
    schedule.reserve(TOTAL_TICKS);
    for (int i = 0; i < TOTAL_TICKS; ++i)
    {
        Instrument instrument = tradeUniverse[i % tradeUniverse.size()];
        MarketEvent ev = {instrument, eventKinds[i]};
        schedule.push_back(ev);
    }

    // Report the realised distribution.
    int seen[EVENT_KIND_COUNT] = {0};
    for (int i = 0; i < TOTAL_TICKS; ++i)
        seen[schedule[i].kind]++;
    std::cout << "\nEvent distribution over " << TOTAL_TICKS << " ticks:" << std::endl;
    for (int k = 0; k < EVENT_KIND_COUNT; ++k)
        std::cout << "  " << eventName((EventKind)k) << ": " << seen[k] << std::endl;

    // Start of demo
    std::cout << "\nStarting balance: " << momentumFund->getFundBalance() << std::endl;
    std::cout << "Running " << TOTAL_TICKS << " market ticks across "
              << tradeUniverse.size() << " instruments...\n"
              << std::endl;

    // benchmark needs a ref
    exchange.setStockPrice(SPX.ticker, SPX.baseline);
    exchange.tick(SPX.ticker, SPX.baseline);

    const int HALF = TOTAL_TICKS / 2;

    // First half
    for (int i = 0; i < HALF; i++)
    {
        const MarketEvent &ev = schedule[i];
        double price = ev.instrument.baseline * eventMultiplier(ev.kind);
        exchange.setStockPrice(ev.instrument.ticker, price);
        exchange.tick(ev.instrument.ticker, price); // notifies deskTrader
    }

    std::cout << "\n midpoint runtime change (tick " << HALF << ") " << std::endl;

    Worker *lateAnalyst = new Worker("late-analyst", KO.ticker, 8); // workerId, ticker, referenceWindow
    std::vector<std::string> lateExtra;
    lateExtra.push_back(NVDA.ticker);
    WatchlistDecorator *lateWatched = new WatchlistDecorator(lateAnalyst, lateExtra);                                                           // wrapped, extraTickers
    BenchmarkComparisonDecorator *lateBenched = new BenchmarkComparisonDecorator(lateWatched, KO.ticker, SPX.ticker, KO.baseline, SPX.baseline); // wrapped, trackedTicker, indexTicker, trackedBaseline, indexBaseline
    momentumFund->add(lateBenched);                                                                                                             // structural change
    std::cout << "  added 'late-analyst' (watchlist + benchmark decorated) to the fund" << std::endl;

    complianceLayer->restrict(PEP.ticker); // behavioural change banning pep
    std::cout << "  PEP is now restricted" << std::endl;
    // show compliance holding it
    for (int t = 0; t < 9; ++t)
        exchange.tick(PEP.ticker, PEP.baseline);
    exchange.tick(PEP.ticker, PEP.baseline * 0.95);
    exchange.tick(PEP.ticker, PEP.baseline * 0.94);
    exchange.tick(PEP.ticker, PEP.baseline * 0.93);
    std::cout << "resuming run \n"
              << std::endl;

    for (int i = HALF; i < TOTAL_TICKS; i++)
    {
        const MarketEvent &ev = schedule[i];
        double price = ev.instrument.baseline * eventMultiplier(ev.kind);
        exchange.setStockPrice(ev.instrument.ticker, price);
        exchange.tick(ev.instrument.ticker, price); // notifies deskTrader
    }

    std::cout << "\nFinal balance: " << momentumFund->getFundBalance() << std::endl;

    // Quick post-run inspection (also exercises the query-side API).
    std::cout << "Last SPX price seen by the exchange: "
              << exchange.getStockPrice(SPX.ticker) << std::endl;

    // Unsubscribe the trader from one ticker 
    exchange.detach(deskTrader, NVDA.ticker);

    // cleanup
    delete deskTrader;
    delete complianceLayer; // deletes riskLayer -> executor
    delete momentumFund;    // deletes the whole desk/analyst/decorator tree

    std::cout << "\n Done" << std::endl;

    return 0;
}
