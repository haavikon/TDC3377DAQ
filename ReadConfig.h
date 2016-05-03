#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <bitset>
#include <sstream>

using namespace std;

class ConfigReader {
public:
	std::string basepath;
	std::string gastarget;
	std::string energy;
	std::string magneticfield;
	std::string gaspressure;
	std::string rawFilename;
	std::string rootFilename;
	std::string metaFilename;
	int CR0 = 0;
	int CR1 = 0;
	int CR2 = 0;
	int CR3 = 0;
	int CR4 = 0;
	int CR5 = 0;
	int TDCMode;
	int GPIBAddress =-1;
	int TDCSlot=-1;
	int DWSSlot=-1;
	std::string filename;
	
	ConfigReader(){}

	ConfigReader(string fName) : filename(fName) { }
	
	void readfile()
	{
		ifstream ifs(filename); //Open filestream
		std::cout << "Reading Config File : " << filename << std::endl;
		string line, sArgument;
		while (getline(ifs, line)) {
			if (line.find("//") == -1)
			{
				istringstream sin(line.substr(line.find(" ") + 1));
				sin >> sArgument;
				action(line, sArgument);
			}
		}
		ifs.close(); // Close filestream
		std:cout << "GPIB Address " << GPIBAddress << " TDC SLot " << TDCSlot << " Dataway Visulizer Slot " << DWSSlot << std::endl;
		bitset<16> x(CR0), y(CR1), z(CR2), u(CR3), v(CR4), w(CR5);
		
		std::cout << "CR0: " << x << std::endl;
		std::cout << "CR1: " << y << std::endl;
		std::cout << "CR2: " << z << std::endl;
		std::cout << "CR3: " << u << std::endl;
		std::cout << "CR4: " << v << std::endl;
		std::cout << "CR5: " << w << std::endl;
	}
private: 
	void action(string line, string argument)
	{
		int iValue = 0;
		iValue = atoi(argument.c_str());
		//put code to do something with the commands in the config file here! 
		if (line.find("BASE_PATH") != -1)
		{
			basepath = argument;
		}

		if (line.find("GAS_TARGET") != -1)
		{
			gastarget = argument;
		}

		if (line.find("GAS_PRESSURE") != -1)
		{
			gaspressure = argument;
		}

		if (line.find("ENERGY") != -1)
		{
			energy = argument;
		}

		if (line.find("MAGNETIC_FIELD") != -1)
		{
			magneticfield = argument;
		}

		if (line.find("GPIB_ADDRESS") != -1)
		{
			GPIBAddress = iValue;
		}

		if (line.find("TDC_SLOT") != -1)
		{
			TDCSlot = iValue;
		}

		if (line.find("DWV_SLOT") != -1)
		{
			DWSSlot = iValue;
		}

		if (line.find("TDC_MODE") != -1)
		{
			if (iValue < 4)
				TDCMode = iValue;
			else
				error("TDC Mode should be less than 4 0 = Common Stop, 1 = Common Start (Single Word), 2 = Common Stop (Double Word), 3 = Common Start (Double Word)");
		}

		if (line.find("MODULE_ID") != -1)
		{
			if (iValue < 256)
				CR0 = CR0 + iValue;
			else
				error("Module ID must be less than 256");
		}

		if (line.find("TDC_RESOLUTION") != -1)
		{
			if (iValue < 4)
				CR0 = CR0 + (iValue << 8);
			else
				error("TDC Resolution is a number between 0 and 3 (0 = 0.5ns, 1 = 1ns, 2 = 2ns, 3 = 3ns) ");
		}

		if (line.find("EDGE") != -1)
		{
			if (iValue == 1)
			{
				CR0 = CR0 + 1024;
			}
			if (iValue > 1 || iValue < 0)
			{
				error("Edge is 1 or 0 (1 = Both edges, 0 = Leading edge)");
				cout << iValue << endl;
			}
		}

		if (line.find("READOUT_MODE") != -1)
		{
			if (iValue == 1)
				CR0 = CR0 + 2048;
			if (iValue>1 || iValue < 0)
				error("Readout Mode is 1 or 0 (1 = ECL Port, 0 = CAMAC)");
		}

		if (line.find("BUFFER_MODE") != -1)
		{
			if (iValue == 1)
				CR0 = CR0 + 4096;
			if (iValue>1 || iValue < 0)
				error("Buffer Mode is 1 or 0 (1 = Multi-Event, 0 = Single)");
		}

		if (line.find("HEADER_MODE") != -1)
		{
			if (iValue == 1)
				CR0 = CR0 + 8192;
			if (iValue>1 || iValue < 0)
				error("Header Mode is 1 or 0 (1 = Skip header with no data words, 0 = Always have header)");
		}

		if (line.find("TRIGGER_OP_PULSE_WIDTH") != -1)
		{
			if (iValue <= 15)
				CR1 = CR1 + iValue;
			else
				error("Trigger output pulse width must be less than 15");
		}


		if (line.find("TRIGGER_PULSE_DELAY") != -1)
		{
			if (iValue <= 15)
				CR1 = CR1 + (iValue << 4);
			else
				error("Trigger pulse delay must be less than 15");
		}

		if (line.find("TRIGGER_CLOCK_UNIT") != -1)
		{
			if (iValue <= 4)
				CR1 = CR1 + (iValue << 8);
			else
				error("Trigger clock unit is a number 0-4 (0 = 25ns, 1 = 5ns, 2 = 1600ns, 3 = external");
		}

		if (line.find("MPI") != -1)
		{
			if (iValue <= 4)
				CR1 = CR1 + (iValue << 10);
			else
				error("MPI is a number 0-4 (0 = no MPI, 1 = 800ns, 2 = 1600ns, 3 = 3200ns");
		}

		if (line.find("FAST_FERA_MODE") != -1)
		{
			if (iValue == 1)
				CR1 = CR1 + 4096;
			if (iValue>1 || iValue < 0)
				error("Fast FERA Mode is 1 or 0 (1 = Fast, 0 = Normal)");
		}

		if (line.find("EVENT_SERIAL_NO") != -1)
		{
			if (iValue <= 7)
				CR1 = CR1 + (iValue << 13);
			else
				error("Event Serial Number must be less than 7");
		}

		if (line.find("HITS_PER_CHANNEL") != -1)
		{
			if (iValue <= 15)
				CR2 = CR2 + iValue;
			else
				error("Hits Per Channel must be less than 15 a value of 0 = 16 hits!");
		}

		if (line.find("MAXIMUM_FULL_SCALE_TIME") != -1)
		{
			if (iValue < 4096)
				CR2 = CR2 + (iValue << 4);
			else
				error("Hits Per Channel must be less than 4095 * 8ns, 0 = 32767.5ns");
		}

		if (line.find("REQUEST_DELAY_SETTING") != -1)
		{
			if (iValue <= 15)
				CR3 = CR3 + iValue;
			else
				error("Request Delay Setting must be less than 15 a value of 0 = 16 hits!");
		}

		if (line.find("OFFSET") != -1)
		{
			if (iValue < 4096)
				CR3 = CR3 + (iValue << 4);
			else
				error("Offset must be less than 4095");
		}

		if (line.find("COM_START_TO_VALUE") != -1)
		{
			if (iValue < 1023)
				CR4 = CR4 + iValue;
			else
				error("Common Start Timeout Value must be less than 1023 * 50ns (0=25ns)");
		}

		if (line.find("TEST_MODE") != -1)
		{
			if (iValue == 1)
				CR5 = CR5 + 256;
			if (iValue>1 || iValue < 0)
				error("Test Mode is 1 or 0 (1 = On, 0 = Off)");
		}

		if (line.find("TEST_MODE_CLOCK") != -1)
		{
			if (iValue < 4)
				CR5 = CR5 + (iValue << 5);
			else
				error("Test mode clock must be 0=100ns, 1 = 200ns, 2=400ns, 3=800ns");
		}

		if (line.find("NO_TEST_PULSES") != -1)
		{
			if (iValue < 32)
				CR5 = CR5 + (iValue);
			else
				error("Number of test pulses must be less than 32");
		}


	}

	void error(string error)
	{
		cout << "ERROR IN CONFIG FILE: " << error << endl;
	}
};
