#include "CooldownState.h"
#include "IdleState.h"
#include "Worker.h"

CooldownState::CooldownState(int ticks)
{
    this->ticksRemaining = ticks;
}

void CooldownState::handleUpdate(Worker *context, double price)
{
    context->recordPrice(price);
    ticksRemaining--;
    if (ticksRemaining <= 0)
    {
        context->setState(new IdleState());
    }
}

std::string CooldownState::name() const
{
    return "Cooldown";
}
