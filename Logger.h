/*=======================================================
Code written by Jacob Hughes
jacob.solomon.hughes@gmail.com
https://github.com/JacobHughes/
=======================================================*/

#pragma once

#include <string>
#include <fstream>
#include <iostream>
#include <ctime>
#include <mutex>
#include <memory>

#include <Windows.h>>



//Different modes for the Logger
enum LoggerMode
{
	LoggerMode_verbose,		//always output to cout
	LoggerMode_silent,		//do not output to cout
	LoggerMode_critical		//only output critical messages
};

enum LoggerLevel
{
	LoggerLevel_normal,		//normal level messages
	LoggerLevel_critical	//critical level messages
};


class Logger
{
	/*
	This is a singleton implementation of a logging class.
	The log() method is thread safe, and thus so is the operator<<
	The Logger maintains two log files, a fullLog (everything is logged here)
	and a critical log.
	Specific messages can be sent to the critical log by using the "critical"
	enum in the log() method.
	The logger can be made "silent" by setting the level. In silent mode, the
	logger will contine to append to log files, but will not output to cout.
	In verbose mode, appending will continue and cout will also be used for each
	message, regardless of LoggerLevel.
	*/
public:
	//Static instance to ensure only one Logger can exist
	static Logger & instance()
	{
		//This creates a small ~720 + ~16 byte memory leak.
		static Logger * singleInstance = new Logger();
		return *singleInstance;
	}

	~Logger()
	{
		//Close and flush all of the logging streams
		if (this->fullLog.is_open())
		{
			this->fullLog.flush();
			this->fullLog.close();
		}

		if (this->criticalLog.is_open())
		{
			this->criticalLog.flush();
			this->criticalLog.close();
		}
	}

	//Function to set the logging mode (verbose or silent)
	inline void setMode(LoggerMode m) { this->mode = m; };

	inline LoggerMode getMode(void) { return this->mode; }

	//operator<< to output to the normal level log file
	template<typename T>
	inline void operator<< (const T &message) { this->log(LoggerLevel_normal, message); };

	//Function to log a message
	template<typename T>
	void log(LoggerLevel level, const T &message)
	{
		//Lock so we don't have output problems
		this->loggerMutex.lock();

		//Create a timestamp
		rawTime = time(0);
		localtime_s(&timeInfo, &rawTime);
		std::string timeStamp = std::to_string(timeInfo.tm_hour) + ":" + std::to_string(timeInfo.tm_min) + ":" + std::to_string(timeInfo.tm_sec) + " ";

		//If this message is critical, write to critical log
		if (level == LoggerLevel_critical)
		{
			SetConsoleTextAttribute(this->hConsole, redOutput);
			criticalLog << timeStamp << message << std::endl;
		}

		//if mode is 0, send to cout
		if (this->mode == LoggerMode_verbose)
		{
			std::cout << timeStamp << message << std::endl;
		}

		//if we want to see only critical message
		if (this->mode == LoggerMode_critical && level == LoggerLevel_critical)
		{
			std::cout << timeStamp << message << std::endl;
		}

		//Always write to full log
		fullLog << timeStamp << message << std::endl;

		SetConsoleTextAttribute(this->hConsole, whiteOutput);
		//Unlock
		this->loggerMutex.unlock();
	}

protected:
	//Constructor is protected / private
	Logger()
	{
		//get output window
		hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		//Start the output as white
		SetConsoleTextAttribute(this->hConsole, whiteOutput);

		//setup directories for logs
		//setUpDirectories();

		//Create a timestamp
		rawTime = time(0);
		localtime_s(&timeInfo, &rawTime);
		std::string dateAddition = std::to_string(timeInfo.tm_year) + "-" + std::to_string(timeInfo.tm_mon) + "-" + std::to_string(timeInfo.tm_mday);

		//Create the full log stream
		while (!fullLog.is_open())
		{
			this->fullLog.open("./logs/LogFile_" + dateAddition + "_fullLog.txt", std::ofstream::out | std::ofstream::app);
		}

		//Create the critical log stream
		while (!criticalLog.is_open())
		{
			this->criticalLog.open("./logs/LogFile_" + dateAddition + "_criticalLog.txt", std::ofstream::out | std::ofstream::app);
		}
	}
	/*
	void setUpDirectories()
	{
		if (directoryExists("./logs/"))
		{
			//log(LoggerLevel_critical, "Directory exists");
		}
		else
		{
			//log(LoggerLevel_critical, "Directory doesn't exist");
			CreateDirectory("./logs/", NULL);
		}
	}

	bool directoryExists(const std::string & directoryName)
	{
		DWORD fileAttributes = GetFileAttributes(directoryName.c_str());

		if (fileAttributes == INVALID_FILE_ATTRIBUTES)
		{
			return false;
		}

		if (fileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			return true;
		}

		return false;
	}*/

	//remove the copy and = methods
	Logger(Logger const&) = delete;
	void operator=(Logger const&) = delete;

	//Set the default logging mode to be verbose
	LoggerMode mode = LoggerMode_verbose;

	//Stream to output everything
	std::ofstream fullLog;

	//Stream to output critical flagged messages
	std::ofstream criticalLog;

	//Time and structures for handling time stamps
	time_t rawTime;
	struct tm timeInfo;
	std::string dateAddition;

	//Mutex for thread safe ofstreams
	std::mutex loggerMutex;

	//windows HANDLE to the console
	HANDLE hConsole;

	//basic white output colour
	int whiteOutput = 15;
	//basic red output colour
	int redOutput = 12;
};

