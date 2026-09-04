#ifndef COMPLIANCEREVIEWDECORATOR_H
#define COMPLIANCEREVIEWDECORATOR_H

class ComplianceReviewDecorator : WorkerDecorator {

public:
	String restrictedList;

	void logDecision(Signal decision);

	void onPriceUpdate();

	void decide();
};

#endif
