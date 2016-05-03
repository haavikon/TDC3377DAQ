#pragma once

#include <string>
#include <ctime>
#include <iostream>
#include <sstream>
#include <windows.h>
#include "ReadConfig.h"

class FNGen {
public:
	std::string basepath;
	std::string path;
	std::string rawFilename;
	std::string rootFilename;
	std::string metaFilename;

	FNGen(ConfigReader &thisReader)
	{
		time_t     now = time(0);
		struct tm  tstruct;
		tstruct = *localtime(&now);					//Get the local time now
		stringstream output;
		basepath = thisReader.basepath;				//Get the base path defined by the config file

		output << basepath << (tstruct.tm_year+1900) << "/ReMi" << tstruct.tm_mday << tstruct.tm_mon << tstruct.tm_hour << tstruct.tm_min << "/";	//Define the file path
		path = output.str();

		output.str("");
		output << "Raw" << tstruct.tm_hour << tstruct.tm_min << tstruct.tm_sec << ".txt";		//DEfine filenames
		rawFilename = path + output.str();

		output.str("");
		output << "Meta" << tstruct.tm_hour << tstruct.tm_min << tstruct.tm_sec << ".txt";
		metaFilename = path + output.str();

		output.str("");
		output << "Tree" << tstruct.tm_hour << tstruct.tm_min << tstruct.tm_sec << ".root";
		rootFilename = path + output.str();
		
		const char* chPath;
		chPath = path.c_str();
		if (!dirExists(chPath))
		{
			wstring temp;
			LPCWSTR dir;

			//CK each directory exists YYYY
			output << basepath << (tstruct.tm_year + 1900) << "/";
			string yeardir = output.str();
			output.str("");
			
			if (!dirExists(yeardir.c_str()))
			{
				temp = s2ws(yeardir);
				dir = temp.c_str();
				CreateDirectoryW(dir, (LPSECURITY_ATTRIBUTES)NULL);
			}

			cout << endl << "Making Directory " << chPath << endl;
			temp = s2ws(path);
			dir = temp.c_str();
			CreateDirectoryW(dir, (LPSECURITY_ATTRIBUTES)NULL);
		}

		ofstream metaStream;	//Write some stuff to a metafile
		cout << metaFilename << std::endl; 
		metaStream.open(metaFilename.c_str());
		metaStream << "Date : " << tstruct.tm_mday << "-" << tstruct.tm_mon << "-" << (tstruct.tm_year+1900) << std::endl;
		metaStream << "Start Time : " << tstruct.tm_hour << ":" << tstruct.tm_min << ":" << tstruct.tm_sec << std::endl;
		metaStream << "Target : " << thisReader.gastarget << std::endl;
		metaStream << "Magnetic Field : " << thisReader.magneticfield << std::endl;
		metaStream << "Gas Pressure : " << thisReader.gaspressure << std::endl;
		metaStream << "Incident Beam Energy : " <<thisReader.energy << std::endl;
		metaStream << endl << "TDC Control Registers" << std::endl;
		metaStream << "CR0 : " << bitset<16>(thisReader.CR0) << std::endl;
		metaStream << "CR1 : " << bitset<16>(thisReader.CR1) << std::endl;
		metaStream << "CR2 : " << bitset<16>(thisReader.CR2) << std::endl;
		metaStream << "CR3 : " << bitset<16>(thisReader.CR3) << std::endl;
		metaStream << "CR4 : " << bitset<16>(thisReader.CR4) << std::endl;
		metaStream << "CR5 : " << bitset<16>(thisReader.CR5) << std::endl;
		metaStream.close();
		thisReader.rawFilename = rawFilename;
		thisReader.rootFilename = rootFilename; 
		thisReader.metaFilename = metaFilename;
		
	}
private:
	int dirExists(const char *path)					//Checks a directory exists
	{
		struct stat info;

		if (stat(path, &info) != 0)
			return 0;
		else if (info.st_mode & S_IFDIR)
			return 1;
		else
			return 0;
	}

	wstring s2ws(const std::string& s)					//Converts std::string to wstring
	{
		int len;
		int slength = (int)s.length() + 1;
		len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
		wchar_t* buf = new wchar_t[len];
		MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
		std::wstring r(buf);
		delete[] buf;
		return r;
	}
};