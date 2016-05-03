#include "Logger.h"
#include "3377TDC.h"
#include "TDCWord.h"
#include "CAMACControler.h"
#include <bitset>
//#include <TTree.h>

LeCroy3377TDC::LeCroy3377TDC()
{
	//nada
}

void LeCroy3377TDC::Init()
{
	int n = Configuration.TDCSlot;
	int iMAXITTERATIONS = 100000;
	Controller.Write(FANCommand(9, 0, n, 0));
	Controller.Write(FANCommand(30, 0, n, 0));
	if (Configuration.TDCMode == 1)
	{
		Controller.Write(FANCommand(21, 0, n, 0));
		cout << "Common Start Single Word Mode" << endl;
	}
	if (Configuration.TDCMode == 2)
	{
		Controller.Write(FANCommand(22, 0, n, 0));
		cout << "Common Stop Double Word Mode" << endl;
	}
	if (Configuration.TDCMode == 3)
	{
		Controller.Write(FANCommand(23, 0, n, 0));
		cout << "Common Start Double Word Mode" << endl;
	}
	Controller.Write(FANCommand(25, 0, n, 0));
	Sleep(500);
	int iCount = 0;
	while ((!Controller.GetQState()) || (iCount == iMAXITTERATIONS))
	{
		Controller.Write(FANCommand(13, 0, n, 0));
		iCount++;
	}
	if (iCount < iMAXITTERATIONS)
	{
		cout << "Xilinx Programming Complete in " << iCount<< " loops" << endl;
	}
	else
	{
		cout << "Error programming Xilinx Chip Exiting..." << endl;
		exit(1);
	}

	Controller.Write(FANCommand(9, 0, n, 0));
	cout << "Writing Control Registers" << endl;
	Controller.Write(FANCommand(17, 0, n, Configuration.CR0));
	Controller.Write(FANCommand(17, 1, n, Configuration.CR1));
	Controller.Write(FANCommand(17, 2, n, Configuration.CR2));
	Controller.Write(FANCommand(17, 3, n, Configuration.CR3));
	Controller.Write(FANCommand(17, 4, n, Configuration.CR4));
	Controller.Write(FANCommand(17, 5, n, Configuration.CR5));
	
	Sleep(200); //almost unneccesary 200ms pause
	
	int CR0, CR1, CR2, CR3, CR4, CR5; 
	CR0 = Controller.Read(FANCommand(1, 0, n, 0));
	CR1 = Controller.Read(FANCommand(1, 1, n, 0));
	CR2 = Controller.Read(FANCommand(1, 2, n, 0));
	CR3 = Controller.Read(FANCommand(1, 3, n, 0));
	CR4 = Controller.Read(FANCommand(1, 4, n, 0));
	CR5 = Controller.Read(FANCommand(1, 5, n, 0));
	
	cout << "Reading back control registers, check with the manual to ensure the configuration is correct!" << endl;
	bitset<16> x(CR0), y(CR1), z(CR2), u(CR3);	// , v(CR4), w(CR5);
	cout << "CR0: " << x << endl << "CR1: " << y << endl << "CR2: " << z << endl << "CR3: " << u << endl; // << "CR4: " << v << endl << "CR5: " << w << endl;

	Controller.Write(FANCommand(9, 0, n, 0));

	Controller.Write(FANCommand(26, 0, n, 0));
	//Should be ready to take data at this point! 
}

void LeCroy3377TDC::ReadCR()
{
	int CR0, CR1, CR2, CR3, CR4, CR5;
	int n = Configuration.TDCSlot;
	CR0 = Controller.Read(FANCommand(1, 0, n, 0));
	CR1 = Controller.Read(FANCommand(1, 1, n, 0));
	CR2 = Controller.Read(FANCommand(1, 2, n, 0));
	CR3 = Controller.Read(FANCommand(1, 3, n, 0));
	CR4 = Controller.Read(FANCommand(1, 4, n, 0));
	CR5 = Controller.Read(FANCommand(1, 5, n, 0));

	cout << "Reading back control registers, check with the manual to ensure the configuration is correct!" << endl;
	bitset<16> x(CR0), y(CR1), z(CR2), u(CR3), v(CR4), w(CR5);
	cout << "CR0: " << x << endl << "CR1: " << y << endl << "CR2: " << z << endl << "CR3: " << u  << endl << "CR4: " << v << endl << "CR5: " << w << endl;
}

int LeCroy3377TDC::ReadData()
{
	int n = Configuration.TDCSlot;
	vector<uint16_t> words;
	while (!Controller.GetQState())
	{
		Controller.Write(FANCommand(27, 2, n, 0));	//Is there Data? 
	}
	words = Controller.BlockRead(FANCommand(0, 0, n, 0));	//Blockread 200 bits of data from the TDC 
	for (auto i = 0; i < words.size(); i++)
	{
		TDCWord thisWord(words[i]);
		if (!thisWord.isHeader)
		{
			if (i + 1 < words.size())
			{
				TDCWord nextWord(words[i + 1]);
				if (thisWord.channel == nextWord.channel)
				{
					int time = thisWord.time + nextWord.time;
					int channel = thisWord.channel;
					int groupnumber = iGroupNumber;
					cout << groupnumber << "\t" << channel << "\t" << time << endl;
				}
			}
		}
		else
		{
			iGroupNumber++;
		}
	}
	return 0;
}

void LeCroy3377TDC::Stop()
{
	int n = Configuration.TDCSlot;
	Controller.Write(FANCommand(24, 1, n, 0));		//Disable Acquisition mode 
	Controller.Write(FANCommand(9, 0, n, 0));		//Clear buffers and LAM
}

void LeCroy3377TDC::Start()
{
	iGroupNumber = -1;
	int n = Configuration.TDCSlot;
	Controller.Write(FANCommand(26, 1, n, 0));							//Enable Acquisition mode 
}