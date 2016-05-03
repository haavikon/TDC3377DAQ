/*=======================================================
	Code written by Jacob Hughes
	
	Copyright (C) Jacob Hughes 2015

	Contact for use:

	yacolt@gmail.com
	https://github.com/JacobHughes/
=======================================================*/

#include "Logger.h"


Logger::Logger()
{
	//Generate a timestamp
	rawTime = time(0);
	localtime_s(&timeInfo, &rawTime);

	char buffer[80];
	strftime(buffer, 80, "%Y-%m-%d", &timeInfo);
	dateAddition = buffer;

	//Create the full log stream
	while (!fullLog.is_open())
	{
		this->fullLog.open(".\\logs\\LogFile_" + dateAddition + "_fullLog.txt", std::ofstream::out | std::ofstream::app);
	}

	//Create the critical log stream
	while (!criticalLog.is_open())
	{
		this->criticalLog.open(".\\logs\\LogFile_" + dateAddition + "_criticalLog.txt", std::ofstream::out | std::ofstream::app);
	}
	//delete[] buffer;
}

Logger::~Logger()
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

void Logger::log(LoggerLevel level, const string &message)
{
	//Lock so we don't have output problems
	this->loggerMutex.lock();

	//Create a timestamp
	time(&rawTime);
	localtime_s(&timeInfo, &rawTime);

	char timeStamp[11];
	strftime(timeStamp, 10, "%H:%M:%S ", &timeInfo);

	
	//If this message is critical, write to critical log
	if (level == LoggerLevel_critical)
	{
		criticalLog << timeStamp << message << endl;
	}
	
	//if mode is 0, send to cout
	if (this->mode == LoggerMode_verbose)
	{
		cout << timeStamp << message << endl;
	}
	
	//Always write to full log
	fullLog << timeStamp << message << endl;
	//delete[] timeStamp;
	
	//Unlock
	this->loggerMutex.unlock();

}