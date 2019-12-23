#include <unordered_map>
#include <map>

enum class JobType
{
	SinglePage = 0,  // Map to false by default
	DoublePage       // Map to true
};

struct JobTypePrice
{
	JobTypePrice(float nonColorPrice, float colorPrice) : m_fNonColorPrice(nonColorPrice), m_fColorPrice(colorPrice) {};
	float m_fNonColorPrice;
	float m_fColorPrice;
};

const static std::unordered_map<JobType, JobTypePrice> sMapPrintJobPrice = { 
	{ JobType::SinglePage, JobTypePrice(0.15f, 0.25f) },
	{ JobType::DoublePage, JobTypePrice(0.1f, 0.2f) }
};

//The class to add separate printJob
class PrintJob
{
public:
	PrintJob(int nTotalPages, int nColorPages, JobType eJobType) : m_nNoneColorPages(nTotalPages - nColorPages)
		, m_nColorPages(nColorPages)
		, m_eJobType(eJobType)
	{
		m_isValidJob = (m_nNoneColorPages < 0 || m_nColorPages < 0) ? false : true;
	}

	bool IsValidJob() { return m_isValidJob; }

	float GetBlackAndWhitePrice() 
	{ 
		if (IsValidJob())
			return sMapPrintJobPrice.at(m_eJobType).m_fNonColorPrice * m_nNoneColorPages;
		else
			return 0.0;
	};

	float GetColorPrice()
	{
		if (IsValidJob())
			return sMapPrintJobPrice.at(m_eJobType).m_fColorPrice * m_nColorPages;
		else
			return 0.0;
	}

	int GetBlackWhitePages() { return m_nNoneColorPages; }
	int GetColorPages() { return m_nColorPages; }
	JobType GetPrintType() { return m_eJobType; }

private:
	int m_nNoneColorPages;
	int m_nColorPages;
	bool m_isValidJob;
	JobType m_eJobType;
};

//The class to create the printer task
class PrinterTask
{
public:
	//Load all print job from a file
	PrinterTask(const std::string& strFileName);

	bool DoCalculate();

	float GetTotalPriceForBlackAndWhite();
	float GetTotalPriceForColor();

	// Return all invalid rows of records
	std::vector<int> GetExceptionLines();

private:
	std::map<int, PrintJob> m_mapRowPrintJobs;
	//Store the print job which has error reading the data
	std::map<int, std::string> m_mapExceptionRows;
	float m_totalPriceBlackAndWhite;
	float m_totalPriceColor;
	std::string m_fileName; 
};