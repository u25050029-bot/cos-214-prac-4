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
// Two independent traversals of the same structure.
//
// fullTraversal    ("full")   visits every work item in the tree and prints its
//                             state and whether it currently has a raised signal.
// signalOnlyTraversal ("signal") visits only the work items whose lifecycle
//                             state reports itself signal-ready, printing those.
//
// Each iterator snapshots the tree's membership at construction, so the two can
// run independently over the same structure without interfering.
// ===========================================================================
static void fullTraversal(WorkItem* root) {
    std::cout << "  [full traversal] every work item:" << std::endl;
    WorkItemIterator* it = root->createIterator("full");
    while (it->hasNext()) {
        std::cout << "      " << it->next()->report() << std::endl;
    }
    delete it;
}

static void signalOnlyTraversal(WorkItem* root) {
    std::cout << "  [signal-ready traversal] only items with a signal:" << std::endl;
    WorkItemIterator* it = root->createIterator("signal");
    bool any = false;
    while (it->hasNext()) {
        any = true;
        std::cout << "      " << it->next()->report() << std::endl;
    }
    if (!any) std::cout << "      (none currently signal-ready)" << std::endl;
    delete it;
}

// ===========================================================================
// Simulated instruments. We trade two primary stocks (NVDA and AMD), a related
// beverage pair (KO / PEP) for the pair-relation strategy, and a market index
// (SPX) used only as a benchmark. Each has its own fixed baseline price.
// ===========================================================================
struct Instrument {
    std::string ticker;
    double baseline;
};

static const Instrument NVDA = { "NVDA", 120.0 };
static const Instrument AMD  = { "AMD",  160.0 };
static const Instrument KO   = { "KO",    60.0 };
static const Instrument PEP  = { "PEP",   55.0 };
static const Instrument SPX  = { "SPX",  5000.0 };

// ===========================================================================
// Market events. Each event is a percentage move applied to an instrument's
// baseline, chosen to drive a particular Worker state transition.
// ===========================================================================
enum EventKind {
    CALM = 0,        // tiny move  -> keeps Idle / drifts Monitoring back to Idle
    STIR,            // ~1.2% move -> Idle -> Monitoring
    RALLY,           // ~3% up     -> Monitoring -> SignalDetected (BUY)
    SELLOFF,         // ~3% down   -> Monitoring -> SignalDetected (SELL)
    HOLD_HIGH,       // stays high -> SignalDetected -> Executing -> Cooldown -> Idle
    HOLD_LOW,        // stays low  -> same, on the SELL side
    EVENT_KIND_COUNT
};

static const char* eventName(EventKind kind) {
    switch (kind) {
        case CALM:      return "CALM";
        case STIR:      return "STIR";
        case RALLY:     return "RALLY";
        case SELLOFF:   return "SELLOFF";
        case HOLD_HIGH: return "HOLD_HIGH";
        case HOLD_LOW:  return "HOLD_LOW";
        default:        return "?";
    }
}

// Convert an event kind into a multiplier on an instrument's baseline.
static double eventMultiplier(EventKind kind) {
    switch (kind) {
        case CALM:      return 1.001;   // +0.1%
        case STIR:      return 1.012;   // +1.2%
        case RALLY:     return 1.030;   // +3.0%
        case SELLOFF:   return 0.970;   // -3.0%
        case HOLD_HIGH: return 1.035;   // hold above the reference
        case HOLD_LOW:  return 0.965;   // hold below the reference
        default:        return 1.000;
    }
}

// A single scheduled market event: which instrument moves, and how.
struct MarketEvent {
    Instrument instrument;
    EventKind  kind;
};

int main() {
    std::cout << "MomentumFund - multi-stock trading simulation" << std::endl;

    // -----------------------------------------------------------------------
    // 1. Build the structure: exchange, fund (composite root), desks, analysts.
    // -----------------------------------------------------------------------
    StockMarket exchange;

    TradingTechnique* momentumFund =
        new TradingTechnique(/* startingBalance */ 1000000.0);

    // --- Technology desk: two analysts on NVDA, one on AMD ---------------
    Division* techDesk =
        new Division(/* tickerFocus */ "TECH");

    Worker* nvidiaLead =
        new Worker(/* workerId */ "nvidia-lead",
                   /* ticker   */ NVDA.ticker,
                   /* referenceWindow */ 8);

    Worker* nvidiaMomentum =
        new Worker(/* workerId */ "nvidia-momentum",
                   /* ticker   */ NVDA.ticker,
                   /* referenceWindow */ 8);
    nvidiaMomentum->setState(new MonitoringState());   // starts already watching closely

    Worker* amdAnalyst =
        new Worker(/* workerId */ "amd-analyst",
                   /* ticker   */ AMD.ticker,
                   /* referenceWindow */ 8);

    // A benchmark-decorated view of the AMD analyst, compared against the index.
    BenchmarkComparisonDecorator* amdVsIndex =
        new BenchmarkComparisonDecorator(/* wrapped         */ amdAnalyst,
                                         /* trackedTicker   */ AMD.ticker,
                                         /* indexTicker     */ SPX.ticker,
                                         /* trackedBaseline */ AMD.baseline,
                                         /* indexBaseline   */ SPX.baseline);

    techDesk->add(nvidiaLead);
    techDesk->add(nvidiaMomentum);
    techDesk->add(amdVsIndex);

    // --- Beverages desk: a KO/PEP pair-relation strategy -----------------
    Division* beveragesDesk =
        new Division(/* tickerFocus */ "BEVERAGES");

    Worker* cokeAnalyst =
        new Worker(/* workerId */ "coke-analyst",
                   /* ticker   */ KO.ticker,
                   /* referenceWindow */ 8);

    Worker* pepsiAnalyst =
        new Worker(/* workerId */ "pepsi-analyst",
                   /* ticker   */ PEP.ticker,
                   /* referenceWindow */ 8);

    beveragesDesk->add(cokeAnalyst);
    beveragesDesk->add(pepsiAnalyst);

    // Wrap the whole beverages desk in a pair-relation strategy on KO vs PEP.
    PairRelationDecorator* beveragesPair =
        new PairRelationDecorator(/* wrapped          */ beveragesDesk,
                                  /* tickerA          */ KO.ticker,
                                  /* tickerB          */ PEP.ticker,
                                  /* historicalSpread */ KO.baseline - PEP.baseline);

    // Assemble the fund: root -> techDesk (+ nested benchmark) and beveragesPair.
    momentumFund->add(techDesk);
    momentumFund->add(beveragesPair);

    // Register every ticker the fund reacts to.
    momentumFund->registerTicker(NVDA.ticker);
    momentumFund->registerTicker(AMD.ticker);
    momentumFund->registerTicker(KO.ticker);
    momentumFund->registerTicker(PEP.ticker);
    momentumFund->registerTicker(SPX.ticker);

    // -----------------------------------------------------------------------
    // 2. Build the decision chain: Compliance -> Risk -> RealTrader.
    // -----------------------------------------------------------------------
    TradeExecutor* executor =
        new RealTrader();

    TradeExecutor* riskLayer =
        new RiskManagementDecorator(/* inner             */ executor,
                                    /* maxTradePct       */ 0.5,
                                    /* maxTickerExposure */ 100.0);

    std::vector<std::string> restrictedTickers;        // no restricted names in this run
    TradeExecutor* complianceLayer =
        new ComplianceReviewDecorator(/* inner          */ riskLayer,
                                      /* technique       */ momentumFund,
                                      /* restrictedList */ restrictedTickers);

    // The desk trader observes the exchange; a tick drives the fund then trades.
    Trader* deskTrader =
        new Trader(/* technique */ momentumFund,
                   /* chain     */ complianceLayer);
    deskTrader->subscribeTo(&exchange);

    // -----------------------------------------------------------------------
    // 3. Define events with a "guarantee" weight (relative frequency).
    // -----------------------------------------------------------------------
    const int TOTAL_TICKS = 200;
    int eventWeight[EVENT_KIND_COUNT];
    eventWeight[CALM]      = 40;
    eventWeight[STIR]      = 25;
    eventWeight[RALLY]     = 12;
    eventWeight[SELLOFF]   = 8;
    eventWeight[HOLD_HIGH] = 10;
    eventWeight[HOLD_LOW]  = 5;
    int weightTotal = 0;
    for (int k = 0; k < EVENT_KIND_COUNT; ++k) weightTotal += eventWeight[k];

    // The instruments that receive events (the two primary stocks plus the
    // beverage pair - the index is driven separately so the benchmark has data).
    std::vector<Instrument> tradeUniverse;
    tradeUniverse.push_back(NVDA);
    tradeUniverse.push_back(AMD);
    tradeUniverse.push_back(KO);
    tradeUniverse.push_back(PEP);

    // -----------------------------------------------------------------------
    // 4. Build the distribution array: TOTAL_TICKS event-kinds by weight.
    // -----------------------------------------------------------------------
    std::vector<EventKind> eventKinds;
    eventKinds.reserve(TOTAL_TICKS);
    for (int k = 0; k < EVENT_KIND_COUNT; ++k) {
        int count = (eventWeight[k] * TOTAL_TICKS) / weightTotal;
        for (int i = 0; i < count; ++i) eventKinds.push_back((EventKind)k);
    }
    for (int k = 0; k < EVENT_KIND_COUNT; ++k) {       // guarantee full coverage
        bool present = false;
        for (std::size_t i = 0; i < eventKinds.size(); ++i)
            if (eventKinds[i] == (EventKind)k) { present = true; break; }
        if (!present) eventKinds.push_back((EventKind)k);
    }
    while ((int)eventKinds.size() < TOTAL_TICKS) eventKinds.push_back(CALM);
    if ((int)eventKinds.size() > TOTAL_TICKS) eventKinds.resize(TOTAL_TICKS);

    // -----------------------------------------------------------------------
    // 5. Shuffle: mix the indexes (Fisher-Yates with a fixed seed).
    // -----------------------------------------------------------------------
    std::srand(2024);
    for (int i = TOTAL_TICKS - 1; i > 0; --i) {
        int j = std::rand() % (i + 1);
        EventKind tmp = eventKinds[i]; eventKinds[i] = eventKinds[j]; eventKinds[j] = tmp;
    }

    // -----------------------------------------------------------------------
    // 6. Process each event-kind into a concrete (instrument, event) schedule,
    //    rotating across the trade universe so both stocks get exercised.
    // -----------------------------------------------------------------------
    std::vector<MarketEvent> schedule;
    schedule.reserve(TOTAL_TICKS);
    for (int i = 0; i < TOTAL_TICKS; ++i) {
        Instrument instrument = tradeUniverse[i % tradeUniverse.size()];
        MarketEvent ev = { instrument, eventKinds[i] };
        schedule.push_back(ev);
    }

    // Report the realised distribution.
    int seen[EVENT_KIND_COUNT] = {0};
    for (int i = 0; i < TOTAL_TICKS; ++i) seen[schedule[i].kind]++;
    std::cout << "\nEvent distribution over " << TOTAL_TICKS << " ticks:" << std::endl;
    for (int k = 0; k < EVENT_KIND_COUNT; ++k)
        std::cout << "  " << eventName((EventKind)k) << ": " << seen[k] << std::endl;

    // -----------------------------------------------------------------------
    // 7. The driving loop: publish each price to the exchange. The Trader, as
    //    an observer, propagates it down the fund (states transition) and then
    //    runs a decision cycle - all triggered by setStockPrice / tick.
    // -----------------------------------------------------------------------
    std::cout << "\nStarting balance: " << momentumFund->getFundBalance() << std::endl;
    std::cout << "Running " << TOTAL_TICKS << " market ticks across "
              << tradeUniverse.size() << " instruments...\n" << std::endl;

    // -----------------------------------------------------------------------
    // Traversal demonstration (before the live run). We build a small desk that
    // is NOT driven by the Trader, hand-drive a couple of analysts into a
    // signal-ready state, and then run the two traversals over it. Because each
    // iterator snapshots the tree at construction, the two coexist independently
    // and a mid-run structural change does not disturb a traversal already made.
    // -----------------------------------------------------------------------
    std::cout << "\n=== Traversal demonstration ===" << std::endl;
    Division* inspectDesk = new Division(/* tickerFocus */ "INSPECT");
    Worker* insA = new Worker(/* workerId */ "insp-A", /* ticker */ "INS", 4);
    Worker* insB = new Worker(/* workerId */ "insp-B", /* ticker */ "INS", 4);
    Worker* insC = new Worker(/* workerId */ "insp-C", /* ticker */ "INS", 4);
    inspectDesk->add(insA);
    inspectDesk->add(insB);
    inspectDesk->add(insC);

    // Drive insA and insB into a raised-signal state (insC stays quiet).
    for (int t = 0; t < 4; ++t) { insA->onPriceUpdate("INS", 100.0); insB->onPriceUpdate("INS", 100.0); insC->onPriceUpdate("INS", 100.0); }
    insA->onPriceUpdate("INS", 103.0);   // move -> Monitoring/SignalDetected
    insA->onPriceUpdate("INS", 104.0);   // enter SignalDetected
    insA->onPriceUpdate("INS", 104.5);   // raise the signal (BUY side)
    insB->onPriceUpdate("INS", 97.0);
    insB->onPriceUpdate("INS", 96.0);    // enter SignalDetected
    insB->onPriceUpdate("INS", 95.5);    // raise the signal (SELL side)

    std::cout << "Full traversal prints every item and its signal status;" << std::endl;
    std::cout << "the signal-only traversal prints just the signal-ready ones.\n" << std::endl;
    fullTraversal(inspectDesk);
    signalOnlyTraversal(inspectDesk);

    // Two independent traversals + a structural change mid-traversal.
    std::cout << "\n  Two iterators are created over the same desk; iterator #1 is\n"
                 "  advanced one step, THEN insC is removed from the desk.\n"
                 "  Iterator #1 keeps its original snapshot; a new iterator #2\n"
                 "  sees the changed structure." << std::endl;
    WorkItemIterator* iter1 = inspectDesk->createIterator("full");
    iter1->next();                                   // advance #1 one step
    inspectDesk->remove(insC);                       // structural change mid-traversal
    delete insC;

    std::cout << "    iterator #1 (snapshot taken before removal) still yields: ";
    int c1 = 1;                                      // already consumed one
    while (iter1->hasNext()) { iter1->next(); c1++; }
    std::cout << c1 << " items" << std::endl;
    delete iter1;

    WorkItemIterator* iter2 = inspectDesk->createIterator("full");
    int c2 = 0; while (iter2->hasNext()) { iter2->next(); c2++; }
    std::cout << "    iterator #2 (created after removal) yields:          "
              << c2 << " items" << std::endl;
    delete iter2;

    delete inspectDesk;   // owns insA, insB (insC already removed + deleted)
    std::cout << "=== end traversal demonstration ===\n" << std::endl;

    // Seed the index once so the benchmark decorator has a reference.
    exchange.setStockPrice(SPX.ticker, SPX.baseline);
    exchange.tick(SPX.ticker, SPX.baseline);

    for (int i = 0; i < TOTAL_TICKS; ++i) {
        const MarketEvent& ev = schedule[i];
        double price = ev.instrument.baseline * eventMultiplier(ev.kind);
        exchange.setStockPrice(ev.instrument.ticker, price);
        exchange.tick(ev.instrument.ticker, price);   // notifies deskTrader
    }

    std::cout << "\nFinal balance: " << momentumFund->getFundBalance() << std::endl;

    // -----------------------------------------------------------------------
    // 8. Cleanup. Ownership: the trader observes and owns nothing; the chain
    //    owns its inner executors; the fund owns the entire composite tree.
    // -----------------------------------------------------------------------
    delete deskTrader;
    delete complianceLayer;   // deletes riskLayer -> executor
    delete momentumFund;      // deletes the whole desk/analyst/decorator tree

    std::cout << "\nSimulation complete." << std::endl;
    return 0;
}
