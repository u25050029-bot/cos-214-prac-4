#include <iostream>
#include <string>
#include <vector>

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

static void banner(const std::string& s) {
    std::cout << "\n========== " << s << " ==========" << std::endl;
}

static void showTree(const std::string& label, WorkItem* root) {
    std::cout << label << ":" << std::endl;
    WorkItemIterator* it = root->createIterator("full");
    while (it->hasNext()) {
        Worker* w = dynamic_cast<Worker*>(it->next());
        if (w) std::cout << "    " << w->getId() << " [" << w->getState()->name() << "]" << std::endl;
    }
    delete it;
}

static void showSignalReady(WorkItem* root) {
    std::cout << "  signal-ready workers:" << std::endl;
    WorkItemIterator* it = root->createIterator("signal");
    bool any = false;
    while (it->hasNext()) {
        Worker* w = dynamic_cast<Worker*>(it->next());
        if (w) { any = true; std::cout << "    " << w->getId() << std::endl; }
    }
    if (!any) std::cout << "    (none)" << std::endl;
    delete it;
}

int main() {
    std::cout << "TaskForge / TradingTechnique - full system demonstration" << std::endl;

    // -----------------------------------------------------------------------
    banner("1. Signal value object");
    // -----------------------------------------------------------------------
    Signal blank;
    std::cout << "  default signal: " << blank.typeName() << " active=" << blank.isActive()
              << " ticker='" << blank.getTicker() << "' qty=" << blank.getQuantity() << std::endl;
    Signal demo("AAPL", SignalType::BUY, "t0", 3.0);
    std::cout << "  built signal: " << demo.typeName() << " " << demo.getTicker()
              << " @" << demo.getTimestamp() << " qty=" << demo.getQuantity()
              << " active=" << demo.isActive() << " type=" << (int)demo.getType() << std::endl;
    demo.setType(SignalType::SELL); demo.setQuantity(9.0); demo.setActive(false);
    std::cout << "  after setters: " << demo.typeName() << " qty=" << demo.getQuantity()
              << " active=" << demo.isActive() << std::endl;

    // -----------------------------------------------------------------------
    banner("2. State pattern - a Worker moving through all five states");
    // -----------------------------------------------------------------------
    {
        Worker probe("probe", "", 3);              // empty-ticker constructor branch
        std::cout << "  empty worker watches AAPL? " << probe.watches("AAPL") << std::endl;
        probe.addTicker("AAPL"); probe.addTicker("AAPL"); // add + duplicate branch
        probe.onPriceUpdate("ZZZZ", 1.0);          // unwatched ticker -> early return
        std::cout << "  reference avg (no data): " << probe.getReferenceAverage() << std::endl;

        WorkerState* same = probe.getState();
        probe.setState(same);                      // setState no-op branch (same pointer)

        Worker w("analyst", "AAPL", 3);
        std::cout << "  start: " << w.getState()->name()
                  << " signalReady=" << w.isSignalReady() << std::endl;
        w.onPriceUpdate("AAPL", 100.0);            // reference 0 -> just records
        w.onPriceUpdate("AAPL", 100.0);
        w.onPriceUpdate("AAPL", 100.0);
        std::cout << "  after flat prices: " << w.getState()->name()
                  << " (ref=" << w.getReferenceAverage() << ", last=" << w.getLastPrice() << ")" << std::endl;
        w.onPriceUpdate("AAPL", 110.0);            // >2% -> Idle transitions to SignalDetected
        std::cout << "  after +10% jump: " << w.getState()->name()
                  << " signalReady=" << w.isSignalReady() << std::endl;
        w.onPriceUpdate("AAPL", 112.0);            // SignalDetected now runs -> raises BUY
        std::cout << "  raised signal: " << w.getSignal().typeName()
                  << " hasSignal=" << w.hasSignal() << std::endl;
        w.onPriceUpdate("AAPL", 113.0);            // already has signal -> no re-raise
        w.clearSignal();
        std::cout << "  after clearSignal: hasSignal=" << w.hasSignal() << std::endl;

        Worker mon("monitor", "MSFT", 5);
        mon.setState(new MonitoringState());
        mon.onPriceUpdate("MSFT", 200.0);
        mon.onPriceUpdate("MSFT", 200.0);
        mon.onPriceUpdate("MSFT", 210.0);          // >1% -> Monitoring transitions
        std::cout << "  monitoring worker now: " << mon.getState()->name() << std::endl;

        Worker exec("executor", "AAPL", 3);
        exec.setState(new ExecutingState());
        exec.onPriceUpdate("AAPL", 500.0);         // records only, stays Executing
        std::cout << "  executing worker: " << exec.getState()->name()
                  << " signalReady=" << exec.isSignalReady()
                  << " last=" << exec.getLastPrice() << std::endl;

        Worker cool("cooling", "AAPL", 3);
        cool.setState(new CooldownState(2));
        cool.onPriceUpdate("AAPL", 10.0);          // 2 -> 1
        std::cout << "  cooldown after 1 tick: " << cool.getState()->name()
                  << " signalReady=" << cool.isSignalReady() << std::endl;
        cool.onPriceUpdate("AAPL", 11.0);          // 1 -> 0 -> Idle
        std::cout << "  cooldown after 2 ticks: " << cool.getState()->name() << std::endl;

        // Worker leaf misc: decide (no-op), balance, both iterator modes
        w.decide();
        std::cout << "  worker balance contribution: " << w.getBalanceContribution() << std::endl;

        // SELL branch: a downward move raises SELL
        Worker down("bear", "AAPL", 3);
        down.onPriceUpdate("AAPL", 100.0);
        down.onPriceUpdate("AAPL", 100.0);
        down.onPriceUpdate("AAPL", 100.0);
        down.onPriceUpdate("AAPL", 80.0);          // transition
        down.onPriceUpdate("AAPL", 79.0);          // raises SELL (price < reference)
        std::cout << "  bear worker raised: " << down.getSignal().typeName() << std::endl;
    }

    // -----------------------------------------------------------------------
    banner("3. Composite pattern - nested Divisions and Workers");
    // -----------------------------------------------------------------------
    Division* techDivision = new Division("AAPL");
    std::cout << "  division focus: " << techDivision->getTickerFocus() << std::endl;
    Worker* a1 = new Worker("A1", "AAPL", 3);
    Worker* a2 = new Worker("A2", "AAPL", 5);
    a2->setState(new MonitoringState());
    Division* energySub = new Division("XOM");
    Worker* e1 = new Worker("E1", "XOM", 3);
    energySub->add(e1);
    techDivision->add(a1);
    techDivision->add(a2);
    techDivision->add(energySub);                  // nested composite
    showTree("  initial tech division", techDivision);
    std::cout << "  aggregate balance: " << techDivision->getBalanceContribution() << std::endl;

    Worker* temp = new Worker("TEMP", "AAPL", 3);
    techDivision->add(temp);
    techDivision->remove(temp);                    // remove hit
    delete temp;
    Worker* stranger = new Worker("STRANGER", "AAPL", 3);
    techDivision->remove(stranger);                // remove miss branch
    delete stranger;
    techDivision->decide();

    // -----------------------------------------------------------------------
    banner("4. Decorator pattern (tree side) - Pair / Watchlist / Benchmark");
    // -----------------------------------------------------------------------
    // Pair: SELL when spread widens above historical
    Division* pairDiv = new Division("KO/PEP");
    pairDiv->add(new Worker("KO", "KO", 3));
    pairDiv->add(new Worker("PEP", "PEP", 3));
    PairRelationDecorator* pair = new PairRelationDecorator(pairDiv, "KO", "PEP", 5.0);
    pair->onPriceUpdate("KO", 50.0);
    pair->onPriceUpdate("PEP", 45.0);              // spread == historical -> no signal
    std::cout << "  pair signal at equal spread? " << pair->hasPairSignal() << std::endl;
    pair->onPriceUpdate("KO", 60.0);               // spread widens -> SELL
    std::cout << "  pair signal after widen?     " << pair->hasPairSignal() << std::endl;
    { std::vector<Signal> out; pair->collectSignals(out);
      std::cout << "  pair collected " << out.size() << " signal(s)" << std::endl; }
    // decorator passthroughs
    { WorkItemIterator* di = pair->createIterator("full");
      int n=0; while (di->hasNext()){di->next();n++;} std::cout << "  pair delegates iterator over " << n << " leaves" << std::endl; delete di; }
    std::cout << "  pair delegates balance=" << pair->getBalanceContribution()
              << " signalReady=" << pair->isSignalReady() << std::endl;

    // Pair BUY branch: spread narrows below historical
    Division* pairDiv2 = new Division("KO/PEP");
    pairDiv2->add(new Worker("KO2", "KO", 3));
    pairDiv2->add(new Worker("PEP2", "PEP", 3));
    PairRelationDecorator* pair2 = new PairRelationDecorator(pairDiv2, "KO", "PEP", 5.0);
    pair2->onPriceUpdate("KO", 50.0);
    pair2->onPriceUpdate("PEP", 44.0);
    pair2->onPriceUpdate("PEP", 60.0);             // spread negative -> BUY
    { std::vector<Signal> out; pair2->collectSignals(out);
      std::cout << "  pair2 (narrowing) first signal: "
                << (out.empty()? "none" : out.back().typeName()) << std::endl; }

    // Watchlist: widen a worker to NVDA
    Worker* watchWorker = new Worker("watcher", "AAPL", 3);
    std::vector<std::string> extra; extra.push_back("NVDA");
    WatchlistDecorator* watch = new WatchlistDecorator(watchWorker, extra);
    std::cout << "  watcher now watches NVDA? " << watchWorker->watches("NVDA") << std::endl;
    watch->onPriceUpdate("NVDA", 100.0);
    watch->onPriceUpdate("NVDA", 100.0);
    watch->onPriceUpdate("NVDA", 100.0);
    watch->onPriceUpdate("NVDA", 130.0);           // reacts via widened coverage
    std::cout << "  watcher state after NVDA jump: " << watchWorker->getState()->name() << std::endl;

    // Benchmark BUY (outperform), SELL (underperform), and parity (no signal)
    Worker* bwUp = new Worker("benchUp", "MSFT", 3);
    BenchmarkComparisonDecorator* benchUp = new BenchmarkComparisonDecorator(bwUp, "MSFT", "SPX", 300.0, 4000.0);
    benchUp->onPriceUpdate("SPX", 4010.0);
    benchUp->onPriceUpdate("MSFT", 315.0);         // outperforms -> BUY
    std::cout << "  benchmark(up) raised? " << benchUp->hasBenchmarkSignal() << std::endl;

    Worker* bwDn = new Worker("benchDn", "MSFT", 3);
    BenchmarkComparisonDecorator* benchDn = new BenchmarkComparisonDecorator(bwDn, "MSFT", "SPX", 300.0, 4000.0);
    benchDn->onPriceUpdate("SPX", 4400.0);
    benchDn->onPriceUpdate("MSFT", 300.0);         // underperforms -> SELL
    { std::vector<Signal> out; benchDn->collectSignals(out);
      std::cout << "  benchmark(down) signal: " << (out.empty()?"none":out.back().typeName()) << std::endl; }

    Worker* bwFlat = new Worker("benchFlat", "MSFT", 3);
    BenchmarkComparisonDecorator* benchFlat = new BenchmarkComparisonDecorator(bwFlat, "MSFT", "SPX", 300.0, 4000.0);
    benchFlat->onPriceUpdate("SPX", 4004.0);
    benchFlat->onPriceUpdate("MSFT", 300.3);        // parity -> no signal
    benchFlat->onPriceUpdate("OTHER", 1.0);         // unrelated ticker + compareGrowth early return
    std::cout << "  benchmark(flat) raised? " << benchFlat->hasBenchmarkSignal() << std::endl;

    Worker* bwZero = new Worker("benchZero", "MSFT", 3);
    BenchmarkComparisonDecorator* benchZero = new BenchmarkComparisonDecorator(bwZero, "MSFT", "SPX", 0.0, 0.0);
    benchZero->onPriceUpdate("SPX", 100.0);
    benchZero->onPriceUpdate("MSFT", 100.0);        // zero-baseline guard
    std::cout << "  benchmark(zero baseline) raised? " << benchZero->hasBenchmarkSignal() << std::endl;

    // -----------------------------------------------------------------------
    banner("5. Observer pattern - StockMarket notifies TradingTechnique");
    // -----------------------------------------------------------------------
    StockMarket market;
    std::cout << "  price of UNSET ticker: " << market.getStockPrice("UNSET") << std::endl;
    market.setStockPrice("AAPL", 101.0);
    std::cout << "  price of AAPL (set):   " << market.getStockPrice("AAPL") << std::endl;

    TradingTechnique* strategy = new TradingTechnique(10000.0);
    strategy->add(techDivision);
    strategy->add(pair);
    strategy->add(pair2);
    strategy->add(watch);
    strategy->add(benchUp);
    strategy->add(benchDn);
    strategy->add(benchFlat);
    strategy->add(benchZero);

    const char* tickers[] = {"AAPL","XOM","KO","PEP","NVDA","MSFT","SPX"};
    for (int i = 0; i < 7; ++i) strategy->registerTicker(tickers[i]);
    strategy->subscribeAll(&market);               // attach

    std::cout << "  feeding market ticks through the observer path..." << std::endl;
    market.tick("AAPL", 100.0);
    market.tick("AAPL", 100.0);
    market.tick("AAPL", 100.0);
    market.tick("AAPL", 108.0);
    market.tick("AAPL", 109.0);                     // second post-transition tick -> raises signal
    market.tick("KO", 55.0);
    showTree("  tree after ticks", strategy);
    showSignalReady(strategy);

    // detach hit + a tick that no longer reaches observers, then detach miss
    market.detach(strategy, "AAPL");
    market.tick("AAPL", 5.0);                       // ignored now
    market.detach(strategy, "GHOST");              // detach miss branch
    std::cout << "  detached AAPL and ticked again (no effect)" << std::endl;

    strategy->adjustBalance(0.0);
    std::cout << "  fund balance: " << strategy->getFundBalance() << std::endl;

    // -----------------------------------------------------------------------
    banner("6. Decorator (decision chain) + Trader");
    // -----------------------------------------------------------------------
    // Standalone RealTrader: HOLD and inactive branches
    RealTrader lone;
    lone.execute(Signal("AAPL", SignalType::HOLD, "now", 0.0));   // HOLD branch
    lone.execute(Signal());                                       // inactive branch

    // Chain: Compliance -> Risk -> RealTrader with restricted list
    TradeExecutor* real = new RealTrader();
    TradeExecutor* risk = new RiskManagementDecorator(real, 0.25, 5.0);
    std::vector<std::string> restricted; restricted.push_back("XOM");
    TradeExecutor* compliance = new ComplianceReviewDecorator(risk, strategy, restricted);

    std::cout << "  balance before manual trades: " << strategy->getFundBalance() << std::endl;
    compliance->execute(Signal("AAPL", SignalType::BUY, "now", 20.0));  // risk shrinks 20->5, commits
    compliance->execute(Signal("XOM", SignalType::BUY, "now", 3.0));    // restricted -> veto to HOLD
    compliance->execute(Signal("AAPL", SignalType::SELL, "now", 1.0));  // SELL commits +100
    std::cout << "  balance after manual trades:  " << strategy->getFundBalance() << std::endl;

    // Risk downgrade-to-HOLD branch (cap 0 shrinks any qty to 0 -> HOLD)
    TradeExecutor* real0 = new RealTrader();
    TradeExecutor* risk0 = new RiskManagementDecorator(real0, 0.25, 0.0);
    std::vector<std::string> noRestrict;
    ComplianceReviewDecorator* comp0 = new ComplianceReviewDecorator(risk0, strategy, noRestrict);
    comp0->execute(Signal("AAPL", SignalType::BUY, "now", 4.0));        // -> downgraded to HOLD
    delete comp0;

    // Trader.runCycle covering combine branches on a purpose-built tree
    banner("6b. Trader decision cycle (combine: SELL override, BUY agree, HOLD, affordability)");
    TradingTechnique* combo = new TradingTechnique(150.0);  // low balance forces affordability skip
    Worker* t1a = new Worker("T1a", "T1", 3);   // T1: two BUYs -> agreement, qty 2 -> notional 200 > 150 skip
    Worker* t1b = new Worker("T1b", "T1", 3);
    t1a->setState(new SignalDetectedState()); t1b->setState(new SignalDetectedState());
    t1a->raiseSignal(SignalType::BUY, 1.0); t1b->raiseSignal(SignalType::BUY, 1.0);
    Worker* t2a = new Worker("T2a", "T2", 3);   // T2: BUY + SELL -> SELL override -> executes
    Worker* t2b = new Worker("T2b", "T2", 3);
    t2a->setState(new SignalDetectedState()); t2b->setState(new SignalDetectedState());
    t2a->raiseSignal(SignalType::BUY, 1.0); t2b->raiseSignal(SignalType::SELL, 1.0);
    Worker* t3 = new Worker("T3", "T3", 3);     // T3: signal-ready but no raised signal -> HOLD combine path
    t3->setState(new ExecutingState());
    combo->add(t1a); combo->add(t1b); combo->add(t2a); combo->add(t2b); combo->add(t3);

    TradeExecutor* creal = new RealTrader();
    TradeExecutor* crisk = new RiskManagementDecorator(creal, 0.5, 100.0);
    std::vector<std::string> cnone;
    TradeExecutor* ccomp = new ComplianceReviewDecorator(crisk, combo, cnone);
    Trader trader(combo, ccomp);
    std::cout << "  balance before cycle: " << combo->getFundBalance() << std::endl;
    trader.runCycle(combo);
    std::cout << "  balance after cycle:  " << combo->getFundBalance()
              << " (T1 BUY skipped as unaffordable, T2 SELL executed)" << std::endl;

    // runCycle edge cases: empty tree and null root
    Division emptyDiv("EMPTY");
    Trader edgeTrader(combo, ccomp);
    edgeTrader.runCycle(&emptyDiv);   // no signals
    edgeTrader.runCycle(nullptr);     // null root guard

    // -----------------------------------------------------------------------
    banner("6c. Remaining edge paths");
    // -----------------------------------------------------------------------
    // Iterator next() past the end -> nullptr (both iterator types)
    Worker* solo = new Worker("solo", "AAPL", 3);
    WorkItemIterator* fit = solo->createIterator("full");   // Worker::createIterator, full
    while (fit->hasNext()) fit->next();
    std::cout << "  full iterator past-end returns null? " << (fit->next() == nullptr) << std::endl;
    delete fit;
    solo->setState(new SignalDetectedState());
    WorkItemIterator* sit = solo->createIterator("signal");  // Worker::createIterator, signal
    while (sit->hasNext()) sit->next();
    std::cout << "  signal iterator past-end returns null? " << (sit->next() == nullptr) << std::endl;
    delete sit;
    delete solo;

    // Trader combine HOLD path: a ticker whose only opinions are HOLD, plus an
    // unaffordable BUY on another ticker (the affordability skip).
    TradingTechnique* holdTree = new TradingTechnique(50.0);  // tiny balance
    Worker* hh = new Worker("HH", "HLD", 3);
    hh->setState(new SignalDetectedState());
    hh->raiseSignal(SignalType::HOLD, 0.0);   // only a HOLD opinion -> combine returns HOLD
    Worker* bb = new Worker("BB", "BUYX", 3);
    bb->setState(new SignalDetectedState());
    bb->raiseSignal(SignalType::BUY, 5.0);    // notional 500 > 50 -> affordability skip
    holdTree->add(hh); holdTree->add(bb);
    TradeExecutor* hreal = new RealTrader();
    Trader holdTrader(holdTree, hreal);
    holdTrader.runCycle(holdTree);            // exercises HOLD-return and affordability continue
    std::cout << "  HOLD-only ticker produced no trade; unaffordable BUY skipped" << std::endl;
    delete hreal;
    delete holdTree;

    // WorkerDecorator::decide via a decorator (base decide -> wrapped->decide)
    Worker* dw = new Worker("decideWorker", "AAPL", 3);
    std::vector<std::string> none2;
    WatchlistDecorator* decDeco = new WatchlistDecorator(dw, none2);
    decDeco->decide();                        // inherited WorkerDecorator::decide
    std::cout << "  decorator decide() delegated to wrapped" << std::endl;
    delete decDeco;                           // WorkerDecorator + WorkerGroup destructors

    // Direct WorkerGroup destruction (group with children deleted directly)
    WorkerGroup* standalone = new Division("STANDALONE");
    standalone->add(new Worker("child1", "AAPL", 3));
    standalone->add(new Worker("child2", "AAPL", 3));
    delete standalone;                        // WorkerGroup::~WorkerGroup body

    // Delete a group and a decorator through a WorkItem* base pointer. This goes
    // through the virtual (deleting) destructor path, covering that variant too.
    WorkItem* groupAsBase = new Division("VIA_BASE");
    std::vector<WorkItem*> flatSink;
    groupAsBase->flatten(flatSink);           // touch a virtual on the base ptr
    delete groupAsBase;
    WorkItem* decoAsBase = new WatchlistDecorator(new Worker("wrapped", "AAPL", 3),
                                                  std::vector<std::string>());
    delete decoAsBase;                        // virtual destructor through base pointer

    // -----------------------------------------------------------------------
    banner("7. Cleanup");
    // -----------------------------------------------------------------------
    delete compliance;   // deletes risk -> real
    delete ccomp;        // deletes crisk -> creal
    delete combo;        // deletes T1a..T3
    delete strategy;     // deletes the whole decorated composite tree

    std::cout << "\nDemonstration complete - all patterns exercised." << std::endl;
    return 0;
}
