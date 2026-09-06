#include "SignalDetectedState.h"
#include "ExecutingState.h"
#include "Worker.h"

void SignalDetectedState::handleUpdate(Worker *context, double price)
{
    double reference = context->getReferenceAverage();
    context->recordPrice(price);

    if (!context->hasSignal())
    {

        SignalType type = (price >= reference) ? SignalType::BUY : SignalType::SELL;
        context->raiseSignal(type, 1.0);
        return;
    }

    SignalType raised = context->getSignal().getType();
    bool stillValid = (raised == SignalType::BUY && price >= reference) ||
                      (raised == SignalType::SELL && price < reference);
    if (stillValid)
    {
        context->setState(new ExecutingState());
    }
}

std::string SignalDetectedState::name() const
{
    return "SignalDetected";
}

bool SignalDetectedState::isSignalReady() const
{
    return true;
}
