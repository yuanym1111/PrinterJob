#include "CsvDataFile.h"
#include <fstream>
#include <algorithm>
#include <atlstr.h>
#include <cstring>
#include <cctype>

using namespace std;
using std::find_if;
using std::for_each;
using std::ifstream;
using std::istream;
using std::ostream;
using std::string;
using std::vector;

// defines value to be used
const char* DEFAULT_DELIMITER = ",";
const char* TRUE_VALUE = "true";
const char* FALSE_VALUE = "false";
static const int   MAX_FIELD_BUFFER = 1024;
// error code table for error reporting
const char* ERROR_REASON[] =
{
	"ERROR 0000: UNKNOWN ERROR",
	"ERROR 0001: An unknown error occurred in GetString()!",
	"ERROR 0002: An unknown error occurred in GetData()!",
	"ERROR 0003: GetString() was called while the content is wrong.",
	"ERROR 0004: The field is not a valid data type."
	"ERROR 0005: Variable name not found!",
	"ERROR 0006: Filename name not found!",
	"ERROR 0007: File not found!",
	"ERROR 0008: The Number of Headers is different than the Number of Data Columns!",
};

// local functions and function objects

// Counts the number of columns in an istream denoted by delim.
static int CountCols(istream& is, const char& delim)
{
	int nCols = 1;
	int iChar;
	do
	{
		if (is.peek() == delim)
			nCols++;
		iChar = is.get();
	} while (iChar != '\n' && iChar != '\r' && iChar != -1);

	return nCols;
}

// trim the space in the begin and end
static void trimSpace(string& strValue)
{
	strValue.erase(strValue.begin(), std::find_if(strValue.begin(), strValue.end(), [](int ch) {
		return !std::isspace(ch);
	}));

	strValue.erase(std::find_if(strValue.rbegin(), strValue.rend(), [](int ch) {
		return !std::isspace(ch);
	}).base(), strValue.end());
}

// A function object designed to be used as a predicate
// for remove_if(). Returns true if names match, false if 
// they don't.
class CompareByName :
	public std::unary_function<std::string, bool>
{
public:
	CompareByName(const char* szName) : m_szName(szName) {}

	bool operator()(const std::string& szVarName) const
	{
		string compareString = szVarName;
		string sourceString = m_szName;
		trimSpace(compareString);
		trimSpace(sourceString);
		return (_stricmp(compareString.c_str(), sourceString.c_str()) == 0);
	}

private:
	std::string m_szName;
};

// end local stuff

// Default constructor
//Do not use it, it is for test purpose
CCsvDataFile::CCsvDataFile()
{
	m_delim = DEFAULT_DELIMITER;
}

// Misc. constructor.  Instantiates an instance of CDataFile and reads the
// specified file.
CCsvDataFile::CCsvDataFile(const char* szFilename)
{
	m_delim = DEFAULT_DELIMITER;
	m_szError = "";

	this->ReadFile(szFilename);
}

// Copy constructor.  Instantiates an instance of CDataFile with the
// contents of another CDataFile.
CCsvDataFile::CCsvDataFile(const CCsvDataFile& df)
{
	*this = df;
}

// destructor
CCsvDataFile::~CCsvDataFile(void)
{
	this->ClearData();
}

// Reads the specified file.
// Returns true if successful, false if an error occurred.
bool CCsvDataFile::ReadFile(const char* szFilename)
{
	try
	{
		m_szFilename = szFilename;

		ifstream inFile;
		inFile.open(szFilename, ifstream::binary | ifstream::in);

		if (inFile.rdstate() & std::ios::failbit)
		{
			inFile.close();
			m_szError = ERROR_REASON[7];
			m_szError += "\nDetails: ";
			m_szError += szFilename;
			return false;
		}

		ReadFromStream(inFile, *this);

		inFile.close();
		return true;
	}

	catch (const exception& e)
	{
		m_szError = e.what();
	}

	catch (...)
	{
		m_szError = ERROR_REASON[0];
	}

	return false;
}

// reads the data from the stream and returns the stream when done.
istream& CCsvDataFile::ReadFromStream(istream& inFile, CCsvDataFile& df)
{
	try
	{
		df.ClearData();

		// Check header
		streampos posData = 0;

		// count the number of variables
		int nVars = CountCols(inFile, df.m_delim.at(0));

		inFile.seekg(posData);

		int iVar = 0;
		int varOffset = df.GetNumberOfVariables();
		int nVarInfo = -1;

		char buff[MAX_FIELD_BUFFER] = { 0 };

		for (iVar = 0; iVar<nVars; iVar++)
		{
			//			inFile.getline(buff, sizeof buff, (iVar == nVars-1) ? '\n' : df.m_delim.at(0));	
			// Changed previous line to the following to correctly support CSV format
			bool bEndOfLine = false;
			ReadCSVstring(inFile, buff, sizeof(buff), (iVar == nVars - 1) ? '\n' : df.m_delim.at(0), bEndOfLine);
			df.m_vstrVariableNames.push_back(buff);
			df.m_vstrSourceFilenames.push_back(df.m_szFilename);
			df.m_v2dStrData.push_back(vector<string>());			
		}

		do
		{
			string strMsg;
			bool bEndOfLine = false;

			for (iVar = 0; iVar<nVars; iVar++)
			{
				//				inFile.getline(buff, sizeof(buff), (iVar == nVars-1) ? '\n' : df.m_delim.at(0));	
				// Changed previous line to the following to correctly support CSV format
				if (!bEndOfLine)
				{
					int iRead = ReadCSVstring(inFile, buff, sizeof(buff), (iVar == nVars - 1) ? '\n' : df.m_delim.at(0), bEndOfLine);

					if (iVar != nVars - 1 && (iRead == 0 || bEndOfLine))
						strMsg = "Line terminated without enough delimiter";

					//we haven't read anything in this line. So, skip it.
					if (iRead == 0)
						break;
				}
				else
					buff[0] = '\0';

				// make sure we didn't pick up extra junk @ eof.
				if (/*!inFile.eof() &&*/ buff[0] != '\n' && buff[0] != '\r')
					df.m_v2dStrData.at(iVar + varOffset).push_back(buff);
			}

			if (!bEndOfLine)
			{
				ReadCSVstring(inFile, buff, sizeof(buff), '\n', bEndOfLine);
				if (strlen(buff) > 0)
					strMsg = "Line contains too many delimiter and data";
			}

			if (nVarInfo != -1)
				df.m_v2dStrData.at(nVarInfo).push_back(strMsg);

		} while (!inFile.eof());

		if (df.m_vstrVariableNames.back().find("\n") != -1)
			df.m_vstrVariableNames.back().resize(df.m_vstrVariableNames.back().length() - 1);

		if (df.m_vstrVariableNames.back().find("\r") != -1)
			df.m_vstrVariableNames.back().resize(df.m_vstrVariableNames.back().length() - 1);
	}

	catch (const exception& e)
	{
		df.m_szError = e.what();
		throw e;
	}

	catch (...)
	{
		df.m_szError = ERROR_REASON[0];
		throw exception(df.m_szError.c_str());
	}

	return inFile;
}

int CCsvDataFile::GetVariableName(const int& iVariable, std::string& rStr)
{
	try
	{
		rStr = m_vstrVariableNames.at(iVariable).c_str();
		return static_cast<int>(rStr.length());
	}

	catch (const exception& e) { m_szError = e.what(); }
	catch (...) { m_szError = ERROR_REASON[1]; }
	return -1;
}

// Clears all data in the CDataFile
void CCsvDataFile::ClearData()
{
	m_szError = "";
	std::vector<std::string>().swap(m_vstrVariableNames);
	std::vector<std::string>().swap(m_vstrSourceFilenames);
	std::vector<std::vector<std::string> >().swap(m_v2dStrData);
}

// Returns the length of the string if successful. 
// Returns -1 if an error is encountered.
int CCsvDataFile::GetData(const int& iVariable, const int& iSample, std::string& rStr)
{
	try
	{
		rStr = m_v2dStrData.at(iVariable).at(iSample).c_str();
		return static_cast<int>(rStr.length());
	}

	catch (const exception& e)
	{
		m_szError = e.what();
		return -1;
	}

	catch (...)
	{
		m_szError = ERROR_REASON[1];
		return -1;
	}
}

// Returns the length of the string if successful. 
// Returns -1 if an error is encountered.
int CCsvDataFile::GetData(const char* szVariableName, const int& iSample, std::string& rStr)
{
	int retVal = 0;

	int index = LookupVariableIndex(szVariableName);

	if (index != -1)
		retVal = GetData(index, iSample, rStr);

	else
	{
		m_szError = ERROR_REASON[5];
		retVal = -1;
	}

	return retVal;
}

// Returns whether get a valid int value from the field
bool CCsvDataFile::GetData(const char* szVariableName, const int& iSample, int& iValue)
{
	// Set default to be zero
	iValue = 0;
	std::string rStr;
	int nLengthStr = GetData(szVariableName, iSample, rStr);

	if (nLengthStr > 0)
	{
		char *numberValidCheck;
		iValue = std::strtol(rStr.c_str(), &numberValidCheck, 10);

		if (*numberValidCheck == '\0')
			return true;
	}
	// If empty string was found in the field, default to be 0
	else if (nLengthStr == 0)
		return true;

	m_szError = ERROR_REASON[4];
	return false;
}

// Returns whether get a bool value from the field
bool CCsvDataFile::GetData(const char* szVariableName, const int& iSample, bool& bValue)
{
	std::string rStr;
	int nLengthStr = GetData(szVariableName, iSample, rStr);

	if (nLengthStr > 0)
	{
		trimSpace(rStr);
		if (_stricmp(rStr.c_str(), TRUE_VALUE) == 0)
			bValue = true;
		else if (_stricmp(rStr.c_str(), FALSE_VALUE) == 0)
			bValue = false;
		else
		{
			m_szError = ERROR_REASON[4];
			return false;
		}
	}
	// If empty string was found in the field, default to be 0
	else if (nLengthStr == 0)
		return false;

	m_szError = ERROR_REASON[3];
	return true;
}

// Returns the index of the first variable name that matches szName.
// Returns -1 if szName is not found.
int CCsvDataFile::LookupVariableIndex(const char* szName, const int& offset /*=0*/) const
{
	int retVal = 0;

	std::vector<std::string>::const_iterator it =
		find_if
		(
		m_vstrVariableNames.begin() + offset,
		m_vstrVariableNames.end(),
		CompareByName(szName)
		);

	if (it == m_vstrVariableNames.end())
		retVal = -1;

	else
	{
		while (it != m_vstrVariableNames.begin())
		{
			retVal++;
			it--;
		}
	}

	return retVal;
}

//********************************************************
// Function:    ReadCSVstring
// Description: Reads an string from an input stream conform CSV specification
//********************************************************
int CCsvDataFile::ReadCSVstring(std::istream& inFile, // input stream to pass
	char* buff,      // buffer to return the value
	int size,        // maximum buffersize passsed
	char delimiter  // what delimiter to be used
	)
{
	bool bEndOfLine;

	return ReadCSVstring(inFile, buff, size, delimiter, bEndOfLine);
}

int CCsvDataFile::ReadCSVstring(std::istream& inFile, // input stream to pass
	char* buff,      // buffer to return the value
	int size,        // maximum buffersize passsed
	char delimiter,  // what delimiter to be used
	bool& bEndOfLine // return if hit end of line
	)
{
	bool quoted = false;     // Is this a quoted string?
	bool backslash = false;  // Is there a backslash?
	int bpos = 0;            // Position within the buffer
	int cRead = 0;           // Characters read sofar from stream
	char cc = 0;             // Current character
	int pc;                 // Peek character

	inFile.get(cc);
	if (cc == 0 || inFile.eof())
	{
		bEndOfLine = true;
		buff[bpos] = '\0';  // terminate string and return
		return 0;       // Read one character and returned...
	}

	cRead++;

	if (cc == '\\')        // Finding a backslash sign...
		backslash = true;

	if (cc == '"')         // Decide is a quoted string and ignore it...
		quoted = true;

	else if (cc == delimiter || cc == '\n' || cc == '\r')
	{
		bEndOfLine = (cc == '\n' || cc == '\r');
		buff[bpos] = '\0';  // terminate string and return

		//if we have CR and next is LF, we read the next char too
		if (cc == '\r' && inFile.peek() == '\n')
		{
			inFile.get(cc);
		}

		return cRead;       // Read one character and returned...
	}
	else
	{
		buff[bpos] = cc;
		bpos++;
	}

	if (inFile.peek() == std::istream::traits_type::eof())
	{
		bEndOfLine = true;
		buff[bpos] = '\0';  // terminate string and return
		return cRead;
	}

	while (!inFile.eof() && (bpos < size))
	{
		inFile.get(cc);     // read next character 

		if (inFile.eof())
			break;

		pc = inFile.peek();

		if ((backslash && cc == 'n') || (backslash && cc == 'r'))
		{
			// convert string '\n' to real new line. 
			// as windows multiline editbox control does not make a new line for just '\n'
			// we need CRLF here
			buff[bpos - 1] = '\r';
			buff[bpos] = '\n';
			bpos++;
			backslash = false;
			continue;
		}

		backslash = (cc == '\\');

		cRead++;
		if (cc == '"' && quoted == true)      // either end of string or an escape quote
		{
			if (delimiter == '\n')
			{
				if (pc == '\n')
				{
					inFile.get(cc); // remove delimiter from the stream
					cRead++;
					break;
				}
				else if (pc == '\r')
				{
					inFile.get(cc); // remove delimiter from the stream
					if (inFile.peek() == '\n')
					{
						inFile.get(cc);
					}
					cRead++;
					break;
				}
				else if (pc == '"')
				{
					inFile.get(cc); // read the "
					cRead++;
				}

			}
			else
			{
				if (pc == delimiter)
				{
					inFile.get(cc); // remove delimiter from the stream
					cRead++;
					break;
				}
				else if (pc == '"')
				{
					inFile.get(cc); // read the "
					cRead++;
				}
			}

			//            else if (isspace(pc)) *** remove spaces after "???
		}

		if (delimiter == '\n')
		{
			if ((cc == '\n' || cc == '\r') && quoted == false)
			{
				if (cc == '\r' && inFile.peek() == '\n')
				{
					inFile.get(cc);
				}

				break;
			}
		}
		else
		{
			if ((cc == delimiter || cc == '\n' || cc == '\r') && quoted == false)
			{
				if (cc == '\r' && inFile.peek() == '\n')
				{
					inFile.get(cc);
				}
				break;
			}
		}


		buff[bpos] = cc;
		bpos++;
		cRead++;
	}

	bEndOfLine = (cc == '\n' || cc == '\r' || inFile.eof());
	buff[bpos] = '\0';  // terminate string and return
	inFile.peek();	// when reach file end, this operation will make inFile.eof return true;
	return cRead;
}