#include <epicsUnitTest.h>

int acalcTest(void);
int scalcTest(void);

void runCalcTests(void)
{
	testHarness();
	
	runTest(acalcTest);
	runTest(scalcTest);
}
