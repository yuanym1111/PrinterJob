// PrinterCalculatror.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "PrintJob.h"

using namespace std;

int main(int argc, char* argv[])
{
	if (argc != 2)
	{
		printf("Usage: PrinterCalculator.exe [filename]");
		return -1;
	}
	unique_ptr<PrinterTask> printTask = make_unique<PrinterTask>(argv[1]);
	if (printTask->DoCalculate())
	{
		printf("Summary:\n");
		printf("Total cost for black and white printing is %.2f\n", printTask->GetTotalPriceForBlackAndWhite());
		printf("Total cost for color printing is %.2f\n", printTask->GetTotalPriceForColor());
	}

	return 0;
}

