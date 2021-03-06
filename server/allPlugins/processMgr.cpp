// screen.cpp : Defines the entry point for the DLL application.
//
#ifndef TRIVAL 
#include "processMgr.h"
#include "conn.h"
#include "macro.h"
#include "svrdll.h"
#include "svrctrl.h"

#include <time.h>
#include <windows.h>
#include <process.h>
#include <tlhelp32.h>
#include <psapi.h>

#pragma comment(lib,"Psapi.lib")

int EnumProcess(conn s, char* buf, int n, writeToClient fn)
{
	HANDLE			hSnapshot = NULL;
	HANDLE			hProcess = NULL;
	HMODULE			hModules = NULL;
	PROCESSENTRY32	pe32 = {0};
	char			strProcessName[MAX_PATH] = {0};
	LPBYTE			lpBuffer = NULL;
	DWORD			dwOffset = 0;
	DWORD			dwLength = 0;
	control_header	header = INITIALIZE_ENCYRPT_HEADER(CONTROL_ENUM_PROCS);
	int				nStatus = 0;
	const int MAX_PACKAGES = 1000 / sizeof(struct process_info);
	struct process_info info[MAX_PACKAGES];	
	int nContent = 0, nSeq = 0;

	EnablePrivilege(SE_DEBUG_NAME, true);

	hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if(hSnapshot == INVALID_HANDLE_VALUE)
	{
		DWORD err = GetLastError();
		if(err == ERROR_ACCESS_DENIED)	nStatus = ERR_DENIED;
		else							nStatus = ERR_NOTFIND;
		goto leave;
	}

	pe32.dwSize = sizeof(PROCESSENTRY32);
	if(Process32First(hSnapshot, &pe32))
	{	  
		do
		{      
			info[nContent].id =  pe32.th32ProcessID;
			strncpy(info[nContent].name, pe32.szExeFile, sizeof(info[nContent].name));
	
			//如果超出一次发送的极限
			nContent++;
			if(nContent == MAX_PACKAGES)
			{
				header.seq++;
				header.dataLen = nContent*sizeof(struct process_info);
				if(fn)
				{
					fn(s, &header, (const char*)&info[0], header.dataLen);
				}
				nContent = 0;
			}
		}
		while(Process32Next(hSnapshot, &pe32));

		//尚且残余一部分数据
		if(nContent != MAX_PACKAGES)
		{
			header.seq++;
			header.dataLen = nContent*sizeof(struct process_info);
			if(fn)
			{
				fn(s, &header, (const char*)&info[0], header.dataLen);
			}
		}

		//告诉客户端数据发送结束
		header.dataLen = -1;
		if(fn)	fn(s, &header, NULL, -1);
	}
	
	EnablePrivilege(SE_DEBUG_NAME, false); 
	CloseHandle(hSnapshot);

	return 0;

leave:
	header.response = nStatus;
	if(fn)	fn(s, &header, NULL, 0);
	return nStatus;
}

int KillProcess(conn s, char* buf, int n, writeToClient fn)
{
	HANDLE hProcess = NULL;
	control_header	header = INITIALIZE_ENCYRPT_HEADER(CONTROL_KILL_PROCS);
	int				nStatus = 0;
	
	EnablePrivilege(SE_DEBUG_NAME, true);

	hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, *(LPDWORD)(buf));
	if(hProcess == INVALID_HANDLE_VALUE || hProcess == NULL)
	{
		DWORD err = GetLastError();
		if(err == ERROR_ACCESS_DENIED)	nStatus = ERR_DENIED;
		else							nStatus = ERR_NOTFIND;
	}
	else
	{
		TerminateProcess(hProcess, 0);
		CloseHandle(hProcess);
	}

	EnablePrivilege(SE_DEBUG_NAME, false);
	
	Sleep(100);
	
	header.response = nStatus;
	if(fn)	fn(s, &header, NULL, 0);
	return nStatus;
}
#endif