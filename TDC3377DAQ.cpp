#include "TDC3377DAQ.h"


//Boolean flag to signal acquisition to continue
std::atomic<boolean> exitProgram;
std::string ConfigFile;

//Function for the thread to execute
unsigned __stdcall threadDataAcquisition(void * p)
{
	ConfigReader thisConfiguration;														//Setup a new config reader class
	thisConfiguration.filename = ConfigFile;											//Give it the config file passed via command line or default
	thisConfiguration.readfile();														//Read the file 
	
	FNGen thisFN(thisConfiguration);
	if ((thisConfiguration.GPIBAddress != -1) & (thisConfiguration.TDCSlot != -1))		//Make sure we have a GPIB address and TDC Slot
	{
		CAMACController thisController;													//Start a CAMAC Controller class
		thisController.Init(thisConfiguration);											//Init CAMAC communcation
		
		LeCroy3377TDC thisTDC;
		thisTDC.Configuration = thisConfiguration;										//Give it the configuration 
		thisTDC.Controller = thisController;											//Give it the GPIB->CAMAC controller 
		thisTDC.Init();																	//Setup the TDC to take data
		thisTDC.Start();																//Start Acquision mode
		while (!exitProgram)
		{ 
			thisTDC.ReadData();															//Read Data 
		}
		thisTDC.Stop();																	//Stop Acquisition mode and clear the buffer
	}
	return 0;
}

int main(int argc, char* argv[])
{
	argc == 1;
	if (argc <= 2)
	{
		if (argc == 1)
		{
			std::cout << "Using default config file" << std::endl;
			ConfigFile = "C:/Users/cams/Documents/Visual Studio 2015/Projects/TDC3377DAQ/Debug/test.cfg";
		}
		else
		{
			ConfigFile = argv[1];
		}

		//Setup DAQ in another thread, this way the TDC can be polled as fast as we like and the console can sit waiting for an input 

		//Parameters to send to the analysis thread.
		int THREAD_PARAMETERS = 1; //If you need to pass the thread any parameters, then change this to the parameters
								   //Set up the single worker thread, start is suspended
		HANDLE acquisitionThread = (HANDLE)_beginthreadex(NULL, 0, &threadDataAcquisition, (LPVOID)THREAD_PARAMETERS, CREATE_SUSPENDED, 0);
		//Resume the thread
		ResumeThread(acquisitionThread);
		char key;
		Logger::instance() << "Press x followed by the enter key to stop data acquistion";
		while (key = std::getchar())
		{
			if (key == 'x')
			{
				//SHUT IT DOWN! 
				exitProgram = true;

				//Wait for the thread to finish
				WaitForSingleObject(acquisitionThread, INFINITE);

				//Exit the while loop
				exit(0);
			}
		}

	}
	if (argc > 2)
	{
		std::cout << "Useage: TDC3377DAQ.exe [CONFIG FILE NAME] e.g. TDC3377DAQ.exe c:\\users\\user\\documents\\test.cfg"<< endl;
		std::cout << "If no argument is passed, the default config file config.cfg in the executable directory will be used"<< endl;
	}
}
