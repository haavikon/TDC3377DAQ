#pragma once

#include <TFile.h>
#include <TTree.h>
#include "CAMACController.h"
#include "ReadConfig.h"

class LeCroy3377TDC {
public:
	CAMACController &Controller;
	ConfigReader &Configuration;
	int iGroupNumber;
	ofstream rawStream;
	shared_ptr<TTree> tree;  
	TFile *hfile;
	int groupnumber;
	int channel;
	int time; 

	LeCroy3377TDC()
	{
		//nada
	}

	void Init()
	{
		int n = Configuration.TDCSlot;
		int iMAXITTERATIONS = 100000;
		Controller.Write(9, 0, n, 0);
		Controller.Write(30, 0, n, 0);
		if (Configuration.TDCMode == 1)
		{
			Controller.Write(21, 0, n, 0);
			std::cout << "Common Start Single Word Mode" << std::endl;
		}
		if (Configuration.TDCMode == 2)
		{
			Controller.Write(22, 0, n, 0);
			std::cout << "Common Stop Double Word Mode" << std::endl;
		}
		if (Configuration.TDCMode == 3)
		{
			Controller.Write(23, 0, n, 0);
			std::cout << "Common Start Double Word Mode" << std::endl;
		}
		Controller.Write(25, 0, n, 0);
		Sleep(500);
		int iCount = 0;
		while ((!Controller.GetQState()) || (iCount == iMAXITTERATIONS))
		{
			Controller.Write(13, 0, n, 0);
			iCount++;
		}
		if (iCount < iMAXITTERATIONS)
		{
			std::cout << "Xilinx Programming Complete in " << iCount << " loops" << std::endl;
		}
		else
		{
			std::cout << "Error programming Xilinx Chip Exiting..." << std::endl;
			exit(1);
		}

		Controller.Write(9, 0, n, 0);
		std::cout << "Writing Control Registers" << std::endl;
		Controller.Write(17, 0, n, Configuration.CR0);
		Controller.Write(17, 1, n, Configuration.CR1);
		Controller.Write(17, 2, n, Configuration.CR2);
		Controller.Write(17, 3, n, Configuration.CR3);
		Controller.Write(17, 4, n, Configuration.CR4);
		Controller.Write(17, 5, n, Configuration.CR5);

		Sleep(200); //almost unneccesary 200ms pause

		int CR0, CR1, CR2, CR3, CR4, CR5;
		CR0 = Controller.Read(1, 0, n);
		CR1 = Controller.Read(1, 1, n);
		CR2 = Controller.Read(1, 2, n);
		CR3 = Controller.Read(1, 3, n);
		CR4 = Controller.Read(1, 4, n);
		CR5 = Controller.Read(1, 5, n);

		std::cout << "Reading back control registers, check with the manual to ensure the configuration is correct!" << std::endl;
		bitset<16> x(CR0), y(CR1), z(CR2), u(CR3);	 , v(CR4), w(CR5);
		std::cout << "CR0: " << x << std::endl << "CR1: " << y << std::endl << "CR2: " << z << std::endl << "CR3: " << u <<  std::endl << "CR4: " << v << std::endl << "CR5: " << w << std::endl;

		Controller.Write(9, 0, n, 0);

		Controller.Write(26, 0, n, 0);
		//Should be ready to take data at this point!

		hfile = 0;
		hfile = TFile::Open(Configuration.rootFilename.c_str(), "RECREATE");
		tree = std::make_shared<TTree>("T", "Data from LeCroy 3377 TDC");	//Setup the tree
		tree->Branch("groupnumber", &groupnumber, "groupnumber/I");		//Setup branches for group number, channel and time
		tree->Branch("channel", &channel, "channel/I");
		tree->Branch("time", &time, "time/I");
	}

	int ReadData()
	{
		int n = Configuration.TDCSlot;
		vector<uint16_t> words;
		while (!Controller.GetQState())
		{
			Controller.Write(27, 2, n, 0);	//Is there Data? 
		}
		words = Controller.BlockRead(0, 0, n);	//Blockread 200 bits of data from the TDC 
		for (auto i = 0; i < words.size(); i++)
		{
			rawStream << words[i] << "\t"; 
			TDCWord thisWord(words[i]);
			if (!thisWord.isHeader)
			{
				if (i + 1 < words.size())
				{
					TDCWord nextWord(words[i + 1]);
					if (thisWord.channel == nextWord.channel)
					{
						time = thisWord.time + nextWord.time;
						channel = thisWord.channel;
						groupnumber = iGroupNumber;
						tree->Fill();
					}
				}
			}
			else
			{
				iGroupNumber++;
			}
		}
		rawStream.flush();
		return 0;
	}

	void Stop()
	{
		int n = Configuration.TDCSlot;
		Controller.Write(24, 1, n, 0);		//Disable Acquisition mode 
		Controller.Write(9, 0, n, 0);		//Clear buffers and LAM
		rawStream.close();
		tree->Print();
		hfile->Write();
		hfile->Close();
		//Get the local time now
		time_t     now = std::time(0);
		struct tm  tstruct;
		tstruct = *localtime(&now);
		ofstream metaStream;	//Write some stuff to a metafile
		metaStream.open(Configuration.metaFilename.c_str(), std::ios::app);
		metaStream << std::endl;
		metaStream << "Stop Date : " << tstruct.tm_mday << "-" << tstruct.tm_mon << "-" << (tstruct.tm_year + 1900) << std::endl;
		metaStream << "Stop Time : " << tstruct.tm_hour << ":" << tstruct.tm_min << ":" << tstruct.tm_sec << std::endl;
		metaStream.close();
	}
	
	void Start()
	{
		tree->SetMaxTreeSize(TREE_MAX_SIZE);
		rawStream.open(Configuration.rawFilename);
		iGroupNumber = -1;
		int n = Configuration.TDCSlot;
		Controller.Write(26, 1, n, 0);							//Enable Acquisition mode 
	}

	void ReadCR()
	{
		int CR0, CR1, CR2, CR3, CR4, CR5;
		int n = Configuration.TDCSlot;
		CR0 = Controller.Read(1, 0, n);
		CR1 = Controller.Read(1, 1, n);
		CR2 = Controller.Read(1, 2, n);
		CR3 = Controller.Read(1, 3, n);
		CR4 = Controller.Read(1, 4, n);
		CR5 = Controller.Read(1, 5, n);

		cout << "Reading back control registers, check with the manual to ensure the configuration is correct!" << endl;
		cout << "CR0: " << bitset<16> (CR0)<< endl << "CR1: " << bitset<16>(CR1) << endl << "CR2: " << bitset<16>(CR2) << endl << "CR3: " << bitset<16>(CR3) << endl << "CR4: " << bitset<16>(CR4) << endl << "CR5: " << bitset<16>(CR5) << endl;
	}
};