#include "MonitoringState.h"
#include "SignalDetectedState.h"
#include "IdleState.h"
#include "Worker.h"
#include <cmath>

void MonitoringState::handleUpdate(Worker* context, double price) {
    double reference = context->getReferenceAverage();
    context->recordPrice(price);

    if (reference > 0.0) {
        double move = std::fabs(price - reference) / reference;

        if (move >= 0.02) {
            context->setState(new SignalDetectedState());
        }

        else if (move < 0.005) {
            context->setState(new IdleState());
        }
    }
}

std::string MonitoringState::name() const {
    return "Monitoring";
}
