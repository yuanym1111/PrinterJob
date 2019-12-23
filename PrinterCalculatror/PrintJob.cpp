#include "PrintJob.h"
#include "CSVDataFile.h"

//Constructor to start loading the CSV file
PrinterTask::PrinterTask(const std::string& strFileName)
	: m_fileName(strFileName)
{
	m_totalPriceColor = 0;
	m_totalPriceBlackAndWhite = 0;
	m_mapRowPrintJobs.clear();
	m_mapExceptionRows.clear();
}

//Actual function to start calculating the printer job
// return true if the task is done
// return false if the task is terminated because of wrong data
bool PrinterTask::DoCalculate()
{
	CCsvDataFile dataFile = CCsvDataFile(m_fileName.c_str());

	if (dataFile.GetLastError()[0] != '\0')
	{
		printf("Meet error when loading the file: %s", dataFile.GetLastError());
		return false;
	}
	int totalRows = dataFile.GetNumberOfSamples(0);
	for (int i = 0; i < totalRows; i++)
	{
		int nTotalPages, nColorPages;
		bool bIsDoulbeSide;
		if (dataFile.GetData("Total Pages", i, nTotalPages)
			&& dataFile.GetData("Color Pages", i, nColorPages)
			&& dataFile.GetData("Double Sided", 0, bIsDoulbeSide))
		{
			PrintJob job(nTotalPages, nColorPages, (JobType)bIsDoulbeSide);
			if (job.IsValidJob())
			{
				std::printf("Add Print Job No. %i - Type: %s, Black and White Printing Pages: %i, Color Printing Pages: %i\n",
					i,
					job.GetPrintType() == JobType::SinglePage ? "single Side" : "Double Side",
					job.GetBlackWhitePages(),
					job.GetColorPages());
				m_totalPriceBlackAndWhite += job.GetBlackAndWhitePrice();
				m_totalPriceColor += job.GetColorPrice();

				m_mapRowPrintJobs.insert(std::pair<int, PrintJob>(i, PrintJob(nTotalPages, nColorPages, (JobType)bIsDoulbeSide)));
			}
		}
		else
		{
			m_mapExceptionRows.insert(std::pair<int, std::string>(i, dataFile.GetLastError()));
			return false;
		}
	}
	return true;
}

float PrinterTask::GetTotalPriceForBlackAndWhite()
{
	return m_totalPriceBlackAndWhite;
}

float PrinterTask::GetTotalPriceForColor()
{
	return m_totalPriceColor;
}

