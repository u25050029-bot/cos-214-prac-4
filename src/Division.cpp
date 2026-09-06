#include "Division.h"

Division::Division(std::string tickerFocus)
{
    this->tickerFocus = tickerFocus;
}

std::string Division::getTickerFocus() const
{
    return tickerFocus;
}
