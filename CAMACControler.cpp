using namespace std;
#include "CAMACControler.h"
//Converted from the c source code in the 8901A manual 

FANCommand::FANCommand(int _f, int _a, int _n, int _data)
{
									//This structure holds the FAN commands for the CAMAC units
	this->f = _f;
	this->a = _a;
	this->n = _n; 
	this->data = _data;
}


CAMACControler::CAMACControler()
{
	//nada
}

void CAMACControler::Write(FANCommand _Command)
{
	char rw[40];					//char array to hold the CAMAC command
	qxRet = 0;						//0 the Q and X responce flag
	ibcmd(gpibBd, pcTalk, 2);		//Set 8901 to Talk
	ibwrt(gpibBd, "d", 1);			//Send 100 --> 24bit read
	ibcmd(gpibBd, "?_", 2);			//Unlisten (63) and Untalk (95) to 8901  
									//Fill the char array with the appropriate command and data 
	rw[0] = (char)_Command.f;	
	rw[1] = (char)_Command.a;
	rw[2] = (char)_Command.n;
	rw[3] = (char)(_Command.data & 0xFFL);
	rw[4] = (char)((_Command.data & 0xFF00L)>>8);
	rw[5] = (char)((_Command.data & 0xFF0000L)>>16);
	ibcmd(gpibBd, pcTalk, 2);		//Set 8901 to talk
	ibwrt(gpibBd, rw, 6);			//Send the command
	ibcmd(gpibBd, pcList, 2);		//Set 8901 to listen
	ibrd(gpibBd, rw, 10);			// Get the Q & X responce after the command
	if (ibcnt == 10)
	{
		qxRet = (int)(rw[3] & 3);
	}

	
	/*stringstream output; 
	output << "F " << _Command.f << " a " << _Command.a << " n " << _Command.n << " data " << _Command.data;
	Logger::instance().log(LoggerLevel_normal, output.str());
	Sleep(200);*/
}

long CAMACControler::Read(FANCommand _Command)
{
	char rw[40];					//char array to hold the CAMAC command
	int d1, d2, d3, result=-1;		//ints to hold the 3 data bits returned by the CAMAC unit and the final result
	qxRet = 0;						//0 the Q and X responce flag
	ibcmd(gpibBd, pcTalk, 2);		//Set the 8901 to talk 
	ibwrt(gpibBd, "d", 1);			//Send 100 --> 24bit read
	ibcmd(gpibBd, "?_", 2);			//Unlisten (63) and Untalk (95) to 8901  
									//Fill the char array with the appropriate command data 
	rw[0] = (char)_Command.f;
	rw[1] = (char)_Command.a;
	rw[2] = (char)_Command.n;
	rw[3] = (char)0;
	rw[4] = (char)0;
	rw[5] = (char)0;
	ibcmd(gpibBd, pcTalk, 2);		//Set 8901 to talk
	ibwrt(gpibBd, rw, 6);			//Send the command
	ibcmd(gpibBd, pcList, 2);		//Set 8901 to listen
	ibrd(gpibBd, rw, 10);			//get responce
	if (ibcnt == 10)
	{
									//Convert the 3 by 3x8 bit numbers into 1x24
		d3 = (long)rw[2];
		d3 = d3 & 0xFF;
		d2 = (long)rw[1];
		d2 = d2 & 0xFF;
		d1 = (long)rw[0];
		d1 = d1 & 0xFF;
		result = (d3 << 16) + (d2 << 8) + d1;
		qxRet = (int)(rw[3] & 3);	//get Q and X responce
	}

	/*
	stringstream output;
	output << "F " << _Command.f << " a " << _Command.a << " n " << _Command.n << " data " << _Command.data;
	Logger::instance().log(LoggerLevel_normal, output.str());
	*/
	return result;
}

void CAMACControler::Init()
{
	int addr = Configuration.GPIBAddress; //Get the GPIB address from the config

	if (gpibBd < 0)					 //checks that the GPIB board was not previously opened then opens it
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

	ibsic(gpibBd);					//Clears GPIB interface

	ibtmo(gpibBd, 12);				//Sets the timeout to 3s
	this->Write(LC8901.CLEAR_and_INIT);			//Send the C&I instruction 
}

vector<uint16_t> CAMACControler::BlockRead(FANCommand _Command)
{
	char rw[40]; 					//char array holds the CAMAC command
									//long* lBuffer[READ_BUFFER_LENGTH];
	int cBuffer[READ_BUFFER_LENGTH];
	vector<uint16_t> words;			//Vector to hold the output
	int _Count = READ_BUFFER_LENGTH * 2; //Read twice the buffer length (possibly because we will read 2*8 bit chars --> 16 bits)
	ibcmd(gpibBd, pcTalk, 2);		//Set the 8901 to Talk Mode
	ibwrt(gpibBd, "j", 1);			//Send 106 --> 16 bit block read 
	ibcmd(gpibBd, "?_", 2);			//Unlisten(63) and Untalk(95) to 8901
	
	//Configure the CAMAC command
	rw[0] = (char)_Command.f;
	rw[1] = (char)_Command.a;
	rw[2] = (char)_Command.n;
	rw[3] = (char)0;
	rw[4] = (char)0;
	rw[5] = (char)0;
	
	ibcmd(gpibBd, pcTalk, 2);		//Set the 8901 to talk mode
	ibwrt(gpibBd, rw, 6);			//Send the CAMAC command
	ibcmd(gpibBd, pcList, 2);		//Read back from the CAMAC box
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
	//words.pop_back();	//Deletes nonsense word off the back of the stack
	return  words;		//return the useful 16 bit words
}

int CAMACControler::qxResp()
{
	return qxRet;					//gets the full Q and X responce, depricated use getXState and getQstate
}

bool CAMACControler::GetXState()
{
	int mask = 1;
	return ((qxRet & mask) == mask);
}

bool CAMACControler::GetQState()
{
	int mask = 2;
	return ((qxRet & mask) == mask);
}
