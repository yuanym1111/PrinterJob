// unittest_printerCalculator.cpp :
//
#include "stdafx.h"
#include "gtest/gtest.h"
#include "CSVDataFile.h"
#include "PrintJob.h"

using namespace std;

TEST(LOADCSVFILE, loadFile)
{
	string content = "Total Pages, Color Pages, Double Sided\n"
		"25, 10,false\n"
		"55, 13, true\n"
		"502, 22, true\n"
		"1, 0, false ";
	CCsvDataFile dataFile;

	dataFile.ReadFromStream(istringstream(content), dataFile);
	//Read file OK
	EXPECT_EQ(dataFile.GetNumberOfSamples(0),4);
	
	//Test read first line
	int nValue;
	EXPECT_TRUE(dataFile.GetData("Total Pages", 0, nValue));
	EXPECT_EQ(nValue, 25);
	EXPECT_TRUE(dataFile.GetData("Color Pages", 0, nValue));
	EXPECT_EQ(nValue, 10);
	//Read false OK
	bool isDoubleSided;
	EXPECT_TRUE(dataFile.GetData("Double Sided", 0, isDoubleSided));
	EXPECT_FALSE(isDoubleSided);

	//Read true OK with blank space in the front
	EXPECT_TRUE(dataFile.GetData("Double Sided", 1, isDoubleSided));
	EXPECT_TRUE(isDoubleSided);

	//Test read last line
	EXPECT_TRUE(dataFile.GetData("Total Pages", 3, nValue));
	EXPECT_EQ(nValue, 1);
	EXPECT_TRUE(dataFile.GetData("Color Pages", 3, nValue));
	EXPECT_EQ(nValue, 0);
	//Read true OK with blank space
	EXPECT_TRUE(dataFile.GetData("Double Sided", 3, isDoubleSided));
	EXPECT_FALSE(isDoubleSided);
}

TEST(LOADCSVFILE, ReadDefaultOrWrongValue)
{
	string content = "Total Pages, Color Pages, Double Sided\n"
		"100,,FALSE\n"
		",,TRUE\n"
		"12ab,cd34,None\n";
	CCsvDataFile dataFile;
	dataFile.ReadFromStream(istringstream(content), dataFile);
	//Read file OK
	EXPECT_EQ(dataFile.GetNumberOfSamples(0), 3);

	//Read first line
	int nValue;
	EXPECT_TRUE(dataFile.GetData("Total Pages", 0, nValue));
	EXPECT_EQ(nValue, 100);
	// Read empty value
	EXPECT_TRUE(dataFile.GetData("Color Pages", 0, nValue));
	EXPECT_EQ(nValue, 0);
	//Read UPPER case
	bool isDoubleSided;
	EXPECT_TRUE(dataFile.GetData("Double Sided", 0, isDoubleSided));
	EXPECT_FALSE(isDoubleSided);

	// Read second line
	// Read empty value
	EXPECT_TRUE(dataFile.GetData("Total Pages", 1, nValue));
	EXPECT_EQ(nValue, 0);
	// Read empty page number
	EXPECT_TRUE(dataFile.GetData("Color Pages", 1, nValue));
	EXPECT_EQ(nValue, 0);
	//Read UPPER case
	EXPECT_TRUE(dataFile.GetData("Double Sided", 1, isDoubleSided));
	EXPECT_TRUE(isDoubleSided);

	//Read wrong bool
	EXPECT_FALSE(dataFile.GetData("Total Pages", 2, nValue));
	EXPECT_FALSE(dataFile.GetData("Total Pages", 2, nValue));
	EXPECT_FALSE(dataFile.GetData("Double Sided", 2, isDoubleSided));
}


TEST(LOADPRINTJOB, ReadSingleSidePrintJob)
{
	PrintJob job(15, 10, JobType::SinglePage);
	EXPECT_TRUE(job.IsValidJob());
	EXPECT_EQ(job.GetBlackWhitePages(), 5);
	EXPECT_EQ(job.GetColorPages(), 10);
	EXPECT_DOUBLE_EQ(job.GetBlackAndWhitePrice(), 5 * sMapPrintJobPrice.at(JobType::SinglePage).m_fNonColorPrice);
	EXPECT_DOUBLE_EQ(job.GetColorPrice(), 10 * sMapPrintJobPrice.at(JobType::SinglePage).m_fColorPrice);
}

TEST(LOADPRINTJOB, ReadDoulbeSidePrintJob)
{
	PrintJob job(25, 10, JobType::DoublePage);
	EXPECT_TRUE(job.IsValidJob());
	EXPECT_EQ(job.GetBlackWhitePages(), 15);
	EXPECT_EQ(job.GetColorPages(), 10);
	EXPECT_DOUBLE_EQ(job.GetBlackAndWhitePrice(), 15 * sMapPrintJobPrice.at(JobType::DoublePage).m_fNonColorPrice);
	EXPECT_DOUBLE_EQ(job.GetColorPrice(), 10 * sMapPrintJobPrice.at(JobType::DoublePage).m_fColorPrice);
}

TEST(LOADPRINTJOB, ReadZeroPageWhiteAndBlackPrintJob)
{
	PrintJob job(15, 15, JobType::SinglePage);
	EXPECT_TRUE(job.IsValidJob());
	EXPECT_EQ(job.GetBlackWhitePages(), 0);
	EXPECT_EQ(job.GetColorPages(), 15);
	EXPECT_DOUBLE_EQ(job.GetBlackAndWhitePrice(), 0);
	EXPECT_DOUBLE_EQ(job.GetColorPrice(), 15 * sMapPrintJobPrice.at(JobType::SinglePage).m_fColorPrice);
}

TEST(LOADPRINTJOB, ReadZeroPageColourPrintJob)
{
	PrintJob job(15, 0, JobType::SinglePage);
	EXPECT_TRUE(job.IsValidJob());
	EXPECT_EQ(job.GetBlackWhitePages(), 15);
	EXPECT_EQ(job.GetColorPages(), 0);
	EXPECT_DOUBLE_EQ(job.GetBlackAndWhitePrice(), 15 * sMapPrintJobPrice.at(JobType::SinglePage).m_fNonColorPrice);
	EXPECT_DOUBLE_EQ(job.GetColorPrice(), 0);
}

TEST(LOADPRINTJOB, ReadInvalidPrintJob)
{
	PrintJob job(15, 16, JobType::SinglePage);
	EXPECT_FALSE(job.IsValidJob());
}


TEST(PRINTTASK, CalculateTotal)
{

}