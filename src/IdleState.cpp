#include "IdleState.h"
#include "MonitoringState.h"
#include "Worker.h"
#include <cmath>

void IdleState::handleUpdate(Worker* context, double price) {
    double reference = context->getReferenceAverage();
    context->recordPrice(price);

    if (reference > 0.0) {
        double move = std::fabs(price - reference) / reference;
        if (move >= 0.01) {
            context->setState(new MonitoringState());
        }
    }
}

std::string IdleState::name() const {
    return "Idle";
}
