#ifndef DIVISION_H
#define DIVISION_H

#include "WorkerGroup.h"

class Division : public WorkerGroup {

private:
	std::string tickerFocus;
public:
	Division(std::string tickerFocus);
};

#endif
