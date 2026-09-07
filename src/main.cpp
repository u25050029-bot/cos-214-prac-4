#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>

#include "Signal.h"
#include "StockMarket.h"
#include "TradingTechnique.h"
#include "WorkerGroup.h"
#include "Division.h"
#include "Worker.h"
#include "IdleState.h"
#include "MonitoringState.h"
#include "SignalDetectedState.h"
#include "ExecutingState.h"
#include "CooldownState.h"
#include "WorkItemIterator.h"
#include "PairRelationDecorator.h"
#include "WatchlistDecorator.h"
#include "BenchmarkComparisonDecorator.h"
#include "TradeExecutor.h"
#include "RealTrader.h"
#include "RiskManagementDecorator.h"
#include "ComplianceReviewDecorator.h"
#include "Trader.h"

// ===========================================================================
// Event model
// ---------------------------------------------------------------------------
// Each event represents the kind of price move that drives a particular state
// transition. Prices are expressed relative to a fixed baseline (100.0) so the
// transition each event triggers is deterministic regardless of ordering.
// ===========================================================================
static const double BASE = 100.0;

enum EventKind {
    EV_CALM = 0,      // < 0.5% move: keeps Idle idle / drifts Monitoring back to Idle
    EV_STIR,          // ~1.2% move: Idle -> Monitoring
    EV_SURGE,         // ~3% up:     Monitoring -> SignalDetected (BUY side)
    EV_PLUNGE,        // ~3% down:   Monitoring -> SignalDetected (SELL side)
    EV_CONFIRM_UP,    // stays high: SignalDetected -> Executing -> Cooldown -> Idle
    EV_CONFIRM_DOWN,  // stays low:  same, on the SELL side
    EV_KIND_COUNT
};

static const char* eventName(EventKind k) {
    switch (k) {
        case EV_CALM:         return "CALM";
        case EV_STIR:         return "STIR";
        case EV_SURGE:        return "SURGE";
        case EV_PLUNGE:       return "PLUNGE";
        case EV_CONFIRM_UP:   return "CONFIRM_UP";
        case EV_CONFIRM_DOWN: return "CONFIRM_DOWN";
        default:              return "?";
    }
}

// Map an event to the price it should push onto the market.
static double eventToPrice(EventKind k) {
    switch (k) {
        case EV_CALM:         return BASE * 1.001;   // +0.1%
        case EV_STIR:         return BASE * 1.012;   // +1.2%
        case EV_SURGE:        return BASE * 1.030;   // +3.0%
        case EV_PLUNGE:       return BASE * 0.970;   // -3.0%
        case EV_CONFIRM_UP:   return BASE * 1.035;   // stay high
        case EV_CONFIRM_DOWN: return BASE * 0.965;   // stay low
        default:              return BASE;
    }
}

int main() {
    std::cout << "TaskForge / TradingTechnique - event-driven simulation" << std::endl;

    // -----------------------------------------------------------------------
    // 1. Create the structure: market, strategy tree, decision chain, trader.
    // -----------------------------------------------------------------------
    StockMarket market;
    TradingTechnique* strategy = new TradingTechnique(1000000.0);

    // A nested composite: root -> techDivision -> equitiesGroup -> Workers,
    // plus a decorated pair branch. All workers track "SIM".
    Division* techDivision = new Division("SIM");
    Worker* a1 = new Worker("A1", "SIM", 8);
    Worker* a2 = new Worker("A2", "SIM", 8);
    a2->setState(new MonitoringState());       // start one worker in Monitoring
    Division* equitiesGroup = new Division("EQ");
    Worker* q1 = new Worker("Q1", "SIM", 8);
    equitiesGroup->add(q1);
    techDivision->add(a1);
    techDivision->add(a2);
    techDivision->add(equitiesGroup);

    // A watchlist-decorated worker widened to SIM.
    Worker* w = new Worker("W1", "OTHER", 8);
    std::vector<std::string> extra; extra.push_back("SIM");
    WatchlistDecorator* watched = new WatchlistDecorator(w, extra);

    strategy->add(techDivision);
    strategy->add(watched);
    strategy->registerTicker("SIM");

    // Decision chain: Compliance -> Risk -> RealTrader.
    TradeExecutor* real = new RealTrader();
    TradeExecutor* risk = new RiskManagementDecorator(real, 0.5, 100.0);
    std::vector<std::string> restricted;                 // none restricted here
    TradeExecutor* chain = new ComplianceReviewDecorator(risk, strategy, restricted);

    // The Trader observes the market; a tick propagates then evaluates.
    Trader* trader = new Trader(strategy, chain);
    trader->subscribeTo(&market);

    // -----------------------------------------------------------------------
    // 2. Define events with a "guarantee" weight (relative frequency).
    // -----------------------------------------------------------------------
    const int N = 200;                    // total number of events in the run
    int weight[EV_KIND_COUNT];
    weight[EV_CALM]         = 40;
    weight[EV_STIR]         = 25;
    weight[EV_SURGE]        = 12;
    weight[EV_PLUNGE]       = 8;
    weight[EV_CONFIRM_UP]   = 10;
    weight[EV_CONFIRM_DOWN] = 5;
    int weightSum = 0;
    for (int k = 0; k < EV_KIND_COUNT; ++k) weightSum += weight[k];

    // -----------------------------------------------------------------------
    // 3. Build the distribution array: N slots filled per the weights.
    // -----------------------------------------------------------------------
    std::vector<EventKind> events;
    events.reserve(N);
    for (int k = 0; k < EV_KIND_COUNT; ++k) {
        int count = (weight[k] * N) / weightSum;
        for (int i = 0; i < count; ++i) events.push_back((EventKind)k);
    }
    // Guarantee at least one of every event kind (full transition coverage),
    // and pad to exactly N with CALM if rounding left us short.
    for (int k = 0; k < EV_KIND_COUNT; ++k) {
        bool present = false;
        for (std::size_t i = 0; i < events.size(); ++i)
            if (events[i] == (EventKind)k) { present = true; break; }
        if (!present) events.push_back((EventKind)k);
    }
    while ((int)events.size() < N) events.push_back(EV_CALM);
    if ((int)events.size() > N) events.resize(N);

    // -----------------------------------------------------------------------
    // 4. Shuffle: mix the indexes (Fisher-Yates with a fixed seed).
    // -----------------------------------------------------------------------
    std::srand(12345);
    for (int i = N - 1; i > 0; --i) {
        int j = std::rand() % (i + 1);
        EventKind tmp = events[i]; events[i] = events[j]; events[j] = tmp;
    }

    // -----------------------------------------------------------------------
    // 5. Process events into price updates.
    // -----------------------------------------------------------------------
    std::vector<double> prices;
    prices.reserve(N);
    for (int i = 0; i < N; ++i) prices.push_back(eventToPrice(events[i]));

    // Report the realised distribution.
    int seen[EV_KIND_COUNT] = {0};
    for (int i = 0; i < N; ++i) seen[events[i]]++;
    std::cout << "\nEvent distribution over " << N << " ticks:" << std::endl;
    for (int k = 0; k < EV_KIND_COUNT; ++k)
        std::cout << "  " << eventName((EventKind)k) << ": " << seen[k] << std::endl;

    // -----------------------------------------------------------------------
    // 6. The driving loop: push each price onto the market. The Trader, as an
    //    observer, propagates it down the tree (states transition) and then
    //    runs a decision cycle - all triggered by setStockPrice/tick.
    // -----------------------------------------------------------------------
    std::cout << "\nStarting balance: " << strategy->getFundBalance() << std::endl;
    std::cout << "Running " << N << " market ticks...\n" << std::endl;

    for (int i = 0; i < N; ++i) {
        market.setStockPrice("SIM", prices[i]);
        market.tick("SIM", prices[i]);      // notifies the Trader -> drive + evaluate
    }

    std::cout << "\nFinal balance: " << strategy->getFundBalance() << std::endl;

    // -----------------------------------------------------------------------
    // 7. Cleanup (clear ownership: trader is an observer and owns nothing here;
    //    chain owns its inner executors; strategy owns the whole tree).
    // -----------------------------------------------------------------------
    delete trader;
    delete chain;
    delete strategy;

    std::cout << "\nSimulation complete." << std::endl;
    return 0;
}
