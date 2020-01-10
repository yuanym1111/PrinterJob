#pragma once
#include <vector>
// the CDataFile class
class CCsvDataFile
{
public:

	//Default constructor.
	CCsvDataFile();

	// Misc. constructor.  Instantiates an instance of CDataFile and reads the
	// specified file with the specified read flags.
	CCsvDataFile(const char* szFilename);

	// Copy constructor.  Instantiates an instance of CDataFile with the
	// contents of another CDataFile.
	CCsvDataFile(const CCsvDataFile& df);

	// Calls ClearData() before the CDataFile is destroyed.
	virtual ~CCsvDataFile(void);

	// Get the int at particular line
	// Return true if the int value is valid
	// Return false if the value is not an Integer
	bool GetData(const char* szVariableName, const int& iSample, int& iValue);

	// Get the bool at particular line
	// Return true if the bool value is valid
	// Return false if the value could not represent as a bool
	bool GetData(const char* szVariableName, const int& iSample, bool& bValue);

	// Assigns the variable name at the specified index to rStr.
	// Returns the new length of rStr if successful, -1 if
	// an error is encountered.
	int GetVariableName(const int& iVariable, std::string& rStr);

	std::istream& ReadFromStream(std::istream& inFile, CCsvDataFile& df);

	// Returns the last error encountered by the class.
	const char* GetLastError() const { return m_szError.c_str(); }

	// Returns the number of variables currently in the CDataFile.
	int GetNumberOfVariables() const { return static_cast<int>(m_vstrVariableNames.size()); }

	// Returns the number of samples currently in the variable.
	int GetNumberOfSamples(const int& iVariable)  const
	{
		return static_cast<int>(m_v2dStrData.at(iVariable).size());
	}

private:
	std::string m_delim;
	std::string m_szFilename;
	std::string m_szError;
	std::vector<std::string> m_vstrVariableNames;
	std::vector<std::string> m_vstrSourceFilenames;
	std::vector<std::vector<std::string> > m_v2dStrData;

	// Private member function for internal bookeeping.

	// Clears the data contained in a CDataFile 
	// and reclaims any allocated memory.
	virtual void ClearData();

	// Reads the specified file with the current read flags.
	// Returns true if successful, false if an error is encountered.
	bool ReadFile(const char* szFilename);

	// Assigns rStr with the data at the target variable.
	// Returns the new length of rStr.  
	// Returns -1 if an error is encountered.
	int GetData(const int&  iVariable, const int& iSample, std::string& rStr);

	// Assigns rStr with the data at the target variable.
	// Returns the new length of rStr.  
	// Returns -1 if an error is encountered.
	int GetData(const char* szVariableName, const int& iSample, std::string& tStr);

	int ReadCSVstring(std::istream& inFile, // input stream to pass
		char* buff,      // buffer to return the value
		int size,        // maximum buffersize passsed
		char delimiter  // what delimiter to be used
		);

	int ReadCSVstring(std::istream& inFile, // input stream to pass
		char* buff,      // buffer to return the value
		int size,        // maximum buffersize passsed
		char delimiter,  // what delimiter to be used
		bool & bEndOfLine
		);

	int LookupVariableIndex(const char* szName, const int& offset = 0) const;
};


