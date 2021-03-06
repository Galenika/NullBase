////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Copyright 2018 Null Terminator																								  //
																																  //
Permission is hereby granted, free of charge, to any person																		  //
obtaining a copy of this software and associated documentation                                                                    //
files(the "Software"), to deal in the Software without restriction,                                                               //
including without limitation the rights to use, copy, modify, merge,                                                              //
publish, distribute, sublicense, and/or sell copies of the Software,                                                              //
and to permit persons to whom the Software is furnished to do so,                                                                 //
subject to the following conditions :                                                                                             //
																																  //
The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.    //
																																  //
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,																	  //
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF																  //
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.                                                            //
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,                                                       //
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR                                                             //
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR                                                             //
THE USE OR OTHER DEALINGS IN THE SOFTWARE.																						  //
*/																																  //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "NullMemory.h"

nullbase::nullbase()
{

}

nullbase::~nullbase()
{
	CloseHandle(hProc);
}

bool nullbase::attatchProc(char* procName)
{
	//Defining size of structure so we can populate it
	procEntry32.dwSize = sizeof(PROCESSENTRY32);

	//Taking a snapshot of all processes running
	hProcSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	if(hProcSnap == INVALID_HANDLE_VALUE)
	{
		std::cout << "Failed to take snapshot of process list." << std::endl;
		return false;
	}

	//While there is a next process in the snapshot
	while(Process32Next(hProcSnap, &procEntry32))
	{
		std::cout << procEntry32.szExeFile << std::endl;
		//If the process we're looking for matches the current process in snapshot
		if (!strcmp(procName, procEntry32.szExeFile))
		{
			//Outputting proc name and ID then getting all access to process and initialising a handle to the proc to we can read/write it's memory
			std::cout << "Found process " << procEntry32.szExeFile << " with process ID " << procEntry32.th32ProcessID << std::endl;
			hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, procEntry32.th32ProcessID);
			pID = procEntry32.th32ProcessID;

			if (hProc == NULL)
			{
				std::cout << "Failed getting handle to process." << std::endl;
			}
			//Close handle to process snapshot then return true
			CloseHandle(hProcSnap);
			return true;
		}
	}

	std::cout << "Couldn't find " << procName << " in the process snapshot." << std::endl;
	CloseHandle(hProcSnap);
	return false;
}

DWORD nullbase::getModule(LPSTR moduleName)
{
	//Taking snapshot of modules in process
	hModuleSnap = INVALID_HANDLE_VALUE;
	hModuleSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pID);

	if(hModuleSnap == INVALID_HANDLE_VALUE)
	{
		std::cout << "Failed to take a snapshot of modules." << std::endl;
		CloseHandle(hModuleSnap);
		return 0;
	}

	//Defining structure size
	modEntry32.dwSize = sizeof(MODULEENTRY32);

	//Getting information of first module in snapshot
	if (Module32First(hModuleSnap, &modEntry32))
	{
		//If the module we're looking for matches the module in snapshot
		if (!strcmp(moduleName, modEntry32.szModule))
		{
			//Outputting module name and base address
			std::cout << "Found module " << modEntry32.szModule << " with base address " << std::hex << (DWORD)modEntry32.modBaseAddr << std::endl;

			//Close handle to process snapshot then return base address
			CloseHandle(hModuleSnap);
			return (DWORD)modEntry32.modBaseAddr;
		}
	}

	//While there is a next module in the snapshot
	while (Module32Next(hModuleSnap, &modEntry32))
	{
		//If the module we're looking for matches the module in snapshot
		if (!strcmp(moduleName, modEntry32.szModule))
		{
			//Outputting module name and base address
			std::cout << "Found module " << modEntry32.szModule << " with base address 0x" << std::hex << std::uppercase << (DWORD)modEntry32.modBaseAddr << std::endl;

			//Close handle to process snapshot then return base address
			CloseHandle(hModuleSnap);
			return (DWORD)modEntry32.modBaseAddr;
		}
	}
	std::cout << "Couldn't find " << moduleName << " module in module snapshot of the process." << std::endl;
	CloseHandle(hModuleSnap);
	return false;
}

DWORD nullbase::getLocalPlayer()
{
	return rpm<DWORD>(baseAddress + offs::dwLocalPlayer);
}

int nullbase::getLocalFlags()
{
	return rpm<int>(getLocalPlayer() + netvars::m_fFlags);
}

int nullbase::getLocalHealth()
{
	std::cout << (getLocalPlayer() + netvars::m_iHealth);
	return rpm<int>(getLocalPlayer() + netvars::m_iHealth);
}

int nullbase::getLocalTeam()
{
	return rpm<int>(getLocalPlayer() + netvars::m_iTeamNum);
}

void nullbase::forceJump()
{
	wpm<int>(1, baseAddress + offs::dwForceJump);
	Sleep(25);
	wpm<int>(0, baseAddress + offs::dwForceJump);
}

bool nullbase::isAlive(DWORD playerBase)
{
	if (getEntHp(playerBase) > 0 && getEntHp(playerBase) < 100)
		return true;
	return false;
}

DWORD nullbase::getEntBase(int index)
{
	return rpm<DWORD>(baseAddress + offs::dwEntityList + (index * 0x10));
}

int	nullbase::getEntHp(DWORD playerBase)
{
	return rpm<int>(playerBase + netvars::m_iHealth);
}

int	nullbase::getEntTeam(DWORD playerBase)
{
	return rpm<int>(playerBase + netvars::m_iTeamNum);
}

int nullbase::getGlowIndex(DWORD playerBase)
{
	return rpm<int>(playerBase + netvars::m_iGlowIndex);
}

DWORD nullbase::getGlowObj()
{
	return rpm<DWORD>(baseAddress + offs::dwGlowObjectManager);
}

bool nullbase::isValid(DWORD playerBase)
{
	if ((isAlive(playerBase) && getEntTeam(playerBase) != 0))
		return true;
	return false;
}

//TODO: MINIMISE WPM CALLS HERE BY USING A GLOW STRUCT 
void nullbase::glowEsp(DWORD glowObj, int glowInd, float r, float g, float b, float a)
{
	wpm<float>(r, (glowObj + ((glowInd * 0x38) + 0x4)));
	wpm<float>(g, (glowObj + ((glowInd * 0x38) + 0x8)));
	wpm<float>(b, (glowObj + ((glowInd * 0x38) + 0xC)));
	wpm<float>(a, (glowObj + ((glowInd * 0x38) + 0x10)));
	wpm<bool>(true, (glowObj + ((glowInd * 0x38) + 0x24)));
	wpm<bool>(false, (glowObj + ((glowInd * 0x38) + 0x25)));
}

bool nullbase::getSpotted(DWORD playerBase)
{
	return rpm<bool>(playerBase + netvars::m_bSpotted);
}

void nullbase::setSpotted(DWORD playerBase, bool val)
{
	wpm<bool>(val, playerBase + netvars::m_bSpotted);
}

int nullbase::getLocalCrossID()
{
	auto temp = rpm<int>(getLocalPlayer() + netvars::m_iCrosshairId);
	if (temp <= 0  || temp > 32)
		return -1;
	return temp;
}




