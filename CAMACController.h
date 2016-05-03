#pragma once

#include <iostream>
#include <bitset>
#include <fstream>
#include <sstream>
#include <string>
#include <Windows.h>
#include <ni488.h>
#include <vector>
#include "GlobalDefines.h"
#include "ReadConfig.h"
#include "TDCWord.h"

enum LeCroy8901ACommands
{
	DisableSRQ = 64,
	EnableSRQ_On_LAM = 65,
	EnableSRQ_On_Q0 = 66,
	EnableSRQ_On_LAM_or_Q0 = 67,
	EnableSRQ_On_X0 = 68,
	EnableSRQ_On_LAM_or_X0 = 69,
	EnableSRQ_On_Q0_or_X0 = 70,
	EnableSRQ_On_LAM_or_Q0_or_X0 = 71,
	//C, Z & I Commands p19 8901A manual
	INITIALIZE = 33,
	CLEAR = 34,
	CLEAR_and_INIT = 35,
	INHIBIT = 72,
	DEINHIBIT = 64,
	//Transfer mode commands P17 8901A manual
	Normal_Transfer_8bit = 97,
	Normal_Transfer_16bit = 98,
	Normal_Transfer_24bit = 100,
	Block_Read_8Bit = 121,
	Block_Read_16Bit = 122,
	Block_Read_24Bit = 124,
	High_Speed_Block_Read_8Bit = 105,
	High_Speed_Block_Read_16Bit = 106,
	High_Speed_Block_Read_24Bit = 108,
};


class CAMACController {
public:

	void Write(int _F, int _A, int _N, int _Data)
	{
		char rw[40];					//char array to hold the CAMAC command
		qxRet = 0;						//0 the Q and X responce flag
		ibcmd(gpibBd, pcTalk, 2);		//Set 8901 to Talk
		ibwrt(gpibBd, "d", 1);			//Send 100 --> 24bit read
		ibcmd(gpibBd, "?_", 2);			//Unlisten (63) and Untalk (95) to 8901  
										//Fill the char array with the appropriate command and data 
		rw[0] = (char)_F;
		rw[1] = (char)_A;
		rw[2] = (char)_N;
		rw[3] = (char)(_Data & 0xFFL);	//split the Data into 24 bits, roll the bits down into 3 by 8 bit numbers
		rw[4] = (char)((_Data & 0xFF00L) >> 8);
		rw[5] = (char)((_Data & 0xFF0000L) >> 16);
		ibcmd(gpibBd, pcTalk, 2);		//Set 8901 to talk
		ibwrt(gpibBd, rw, 6);			//Send the command
		ibcmd(gpibBd, pcList, 2);		//Set 8901 to listen
		ibrd(gpibBd, rw, 10);			// Get the Q & X responce after the command
		if (ibcnt == 10)
		{
			qxRet = (int)(rw[3] & 3);
		}
	}

	void CAMAC_Command(LeCroy8901ACommands _CAMACCommand)
	{
		char rw[40];					//char array to hold the CAMAC command
		qxRet = 0;						//0 the Q and X responce flag
		ibcmd(gpibBd, pcTalk, 2);		//Set 8901 to Talk
		ibwrt(gpibBd, "d", 1);			//Send 100 --> 24bit read
		ibcmd(gpibBd, "?_", 2);			//Unlisten (63) and Untalk (95) to 8901  
										//Fill the char array with the appropriate command and data 
		rw[0] = (char)_CAMACCommand;
		rw[1] = (char)0;
		rw[2] = (char)0;
		rw[3] = (char)0;
		rw[4] = (char)0;
		rw[5] = (char)0;
		ibcmd(gpibBd, pcTalk, 2);		//Set 8901 to talk
		ibwrt(gpibBd, rw, 6);			//Send the command
		ibcmd(gpibBd, pcList, 2);		//Set 8901 to listen
		ibrd(gpibBd, rw, 10);			// Get the Q & X responce after the command
		if (ibcnt == 10)
		{
			qxRet = (int)(rw[3] & 3);
		}
	}


	long Read(int _F, int _A, int _N)
	{
		char rw[40];								//char array to hold the CAMAC command
		int d1, d2, d3, result = -1;				//ints to hold the 3 data bits returned by the CAMAC unit and the final result
		qxRet = 0;									//0 the Q and X responce flag
		ibcmd(gpibBd, pcTalk, 2);					//Set the 8901 to talk 
		ibwrt(gpibBd, "d", 1);						//Send 100 --> 24bit read
		ibcmd(gpibBd, "?_", 2);						//Unlisten (63) and Untalk (95) to 8901  
													//Fill the char array with the appropriate command data 
		rw[0] = (char)_F;
		rw[1] = (char)_A;
		rw[2] = (char)_N;
		rw[3] = (char)0;
		rw[4] = (char)0;
		rw[5] = (char)0;
		ibcmd(gpibBd, pcTalk, 2);					//Set 8901 to talk
		ibwrt(gpibBd, rw, 6);						//Send the command
		ibcmd(gpibBd, pcList, 2);					//Set 8901 to listen
		ibrd(gpibBd, rw, 10);						//get responce
		if (ibcnt == 10)
		{
													//Convert the 3 by 3x8 bit numbers into 1x24
			d3 = (long)rw[2];
			d3 = d3 & 0xFF;
			d2 = (long)rw[1];
			d2 = d2 & 0xFF;
			d1 = (long)rw[0];
			d1 = d1 & 0xFF;
			result = (d3 << 16) + (d2 << 8) + d1;	//roll the 3 by 8 bit numbers into a 24 bit number (3 bytes)
			qxRet = (int)(rw[3] & 3);				//get Q and X responce
		}
		return result;
	}
	
	void Init(ConfigReader &_thisConfiguration)
	{				
		int addr = _thisConfiguration.GPIBAddress;					//Get the GPIB address from the config

		if (gpibBd < 0)												//checks that the GPIB board was not previously opened then opens it
		{
			if ((gpibBd = ibfindA("GPIB0"))<0)
				return;
		}

		if (addr > 0 && addr < 29)
		{
																	//setup the commands to switch GPIB to talk and listen
			pcTalk[1] = 0x20 + addr;
			pcList[1] = 0x40 + addr;
		}

		ibsic(gpibBd);												//Clears GPIB interface

		ibtmo(gpibBd, 12);											//Sets the timeout to 3s
		CAMAC_Command(LeCroy8901ACommands::CLEAR_and_INIT);			//Send the C&I instruction 
	}



	vector<uint16_t> BlockRead(int _F, int _A, int _N)
	{
		char rw[40]; 							//char array holds the CAMAC command
												//long* lBuffer[READ_BUFFER_LENGTH];
		int cBuffer[READ_BUFFER_LENGTH];
		vector<uint16_t> words;					//Vector to hold the output
		int _Count = READ_BUFFER_LENGTH * 2;	//Read twice the buffer length (possibly because we will read 2*8 bit chars --> 16 bits)
		ibcmd(gpibBd, pcTalk, 2);				//Set the 8901 to Talk Mode
		ibwrt(gpibBd, "j", 1);					//Send 108 --> 16 bit block read 
		ibcmd(gpibBd, "?_", 2);					//Unlisten(63) and Untalk(95) to 8901
												//Configure the CAMAC command
		rw[0] = (char)_F;
		rw[1] = (char)_A;
		rw[2] = (char)_N;
		rw[3] = (char)0;
		rw[4] = (char)0;
		rw[5] = (char)0;

		ibcmd(gpibBd, pcTalk, 2);				//Set the 8901 to talk mode
		ibwrt(gpibBd, rw, 6);					//Send the CAMAC command
		ibcmd(gpibBd, pcList, 2);				//Read back from the CAMAC box
		int returnValue = ibrd(gpibBd, (char*)cBuffer, _Count);	//fill the buffer with all the returned data

		for (auto i = 0; i < READ_BUFFER_LENGTH; i++) //Loop over the entire buffer
		{
			uint32_t number = (uint32_t)cBuffer[i];	//This read in integers so lets stick with 32 unsigned bits 
			uint32_t d1, d2, d3, d4;
			//okay, lets pull those useful bytes of information out
			d1 = (number & 255);
			d2 = (number & 65280) >> 8;
			d3 = (number & 16711680) >> 16;
			d4 = (number & 4278190080) >> 24;
			//combine the bytes into 16 bit data words
			uint16_t Word1 = (d2 << 8) + d1;
			uint16_t Word2 = (d4 << 8) + d3;
			//there is usually a 0 bit followed by gibberish from memory, so once you hit a 0 stop
			if (Word1 > 0)
			{
				words.push_back(Word1);
			}
			else
			{
				break;
			}
			if (Word2 > 0)
			{
				words.push_back(Word2);
			}
			else
			{
				break;
			}
		}
		return  words;		//return the useful 16 bit words
	}

	bool GetQState()
	{
		int mask = 2;
		return ((qxRet & mask) == mask);
	}

	bool GetXState()
	{
		int mask = 1;
		return ((qxRet & mask) == mask);
	}

private:
	int qxRet;
	int gpibBd=-1;
	char pcTalk[3]="@!";
	char pcList[3]=" A";
};




