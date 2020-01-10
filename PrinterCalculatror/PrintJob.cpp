#include "stdafx.h"
#include "PrintJob.h"

//Constructor to start loading the CSV file by file name
PrinterTask::PrinterTask(const std::string& strFileName)
{
	m_ptrCsvFile = std::make_unique<CCsvDataFile>(strFileName.c_str());
	m_totalPriceColor = 0;
	m_totalPriceBlackAndWhite = 0;
	m_mapRowPrintJobs.clear();
	m_mapExceptionRows.clear();
}

//Constructor to start loading the CSV file by the file reference
PrinterTask::PrinterTask(std::unique_ptr<CCsvDataFile> df)
{
	m_ptrCsvFile = std::move(df);
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
	assert(m_ptrCsvFile);
	if (m_ptrCsvFile->GetLastError()[0] != '\0')
	{
		printf("Meet error when loading the file: %s", m_ptrCsvFile->GetLastError());
		return false;
	}
	int totalRows = m_ptrCsvFile->GetNumberOfSamples(0);
	for (int i = 0; i < totalRows; i++)
	{
		int nTotalPages, nColorPages;
		bool bIsDoulbeSide;
		if (m_ptrCsvFile->GetData("Total Pages", i, nTotalPages)
			&& m_ptrCsvFile->GetData("Color Pages", i, nColorPages)
			&& m_ptrCsvFile->GetData("Double Sided", i, bIsDoulbeSide))
		{
			PrintJob job(nTotalPages, nColorPages, (JobType)bIsDoulbeSide);
			if (job.IsValidJob())
			{
				std::printf("Add Print Job No. %i - Type: %s\n Black and White Printing Pages: %i, cost: %.2f\n Color Printing Pages: %i, cost: %.2f\n\n ",
					i,
					job.GetPrintType() == JobType::SinglePage ? "single Side" : "Double Side",
					job.GetBlackWhitePages(),
					job.GetBlackAndWhitePrice(),
					job.GetColorPages(),
					job.GetColorPrice());
				m_totalPriceBlackAndWhite += job.GetBlackAndWhitePrice();
				m_totalPriceColor += job.GetColorPrice();

				m_mapRowPrintJobs.insert(std::pair<int, PrintJob>(i, PrintJob(nTotalPages, nColorPages, (JobType)bIsDoulbeSide)));
			}
		}
		else
		{
			m_mapExceptionRows.insert(std::pair<int, std::string>(i, m_ptrCsvFile->GetLastError()));
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

