#include "interpret.h"
#include <iostream>

using namespace cfvi::interpretation;

int main() {

	interpreter i = interpreter();
	i.import({ "account/secure_data/.clearance" });

	interpreter i2 = interpreter();
	i2.import({"account/.clearance"});

	std::cout << "\n\n\n\n\n\n\n";

	return 0;
}
