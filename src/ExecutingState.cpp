#include "ExecutingState.h"
#include "CooldownState.h"
#include "Worker.h"

ExecutingState::ExecutingState(int workDuration)
    : ticksWorked(0), workDuration(workDuration) {}

void ExecutingState::handleUpdate(Worker* context, double price) {
    context->recordPrice(price);
    ticksWorked++;

    if (ticksWorked >= workDuration) {
        context->clearSignal();
        context->setState(new CooldownState());
    }
}

std::string ExecutingState::name() const {
    return "Executing";
}

bool ExecutingState::isSignalReady() const {
    return true;
}
