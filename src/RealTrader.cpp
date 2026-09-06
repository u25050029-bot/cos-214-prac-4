#include "RealTrader.h"
#include <iostream>

RealTrader::RealTrader() {}

Signal RealTrader::execute(Signal decision)
{

    if (!decision.isActive() || decision.getType() == SignalType::HOLD)
    {
        std::cout << "[Trader] No trade to settle for " << decision.getTicker() << std::endl;
        return decision;
    }

    std::cout << "[Trader] Settled " << decision.typeName() << " " << decision.getTicker()
              << " qty=" << decision.getQuantity() << " (pending compliance)" << std::endl;
    return decision;
}
