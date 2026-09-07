#include "WorkItem.h"

bool WorkItem::isSignalReady() const
{
    return false;
}

std::string WorkItem::report() const
{
    return "(node)";
}

void WorkItem::addWatchTicker(const std::string &ticker)
{
    (void)ticker;

}

std::vector<std::string> WorkItem::getTickers() const
{
    return std::vector<std::string>();
}

void WorkItem::consumeOwnSignal(std::vector<Signal> &out)
{
    (void)out;
}
