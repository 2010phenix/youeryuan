// PluginApp.cpp: implementation of the PluginApp class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PluginApp.h"
#include "pluginMgr.h"
#include "DlgRegister.h"
#include "SEU_QQwry.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(PluginApp, CWinApp)
	ON_COMMAND_RANGE(WM_PLUG_FIRST, WM_PLUG_LAST, OnPlugCmd)
	ON_UPDATE_COMMAND_UI_RANGE(WM_PLUG_FIRST, WM_PLUG_LAST, OnPlugCmdUI)
END_MESSAGE_MAP()

PluginApp::PluginApp()
{

}

PluginApp::~PluginApp()
{

}

static void GetApplicationPath(char* appPath, int n)
{
	char filename[MAX_PATH], drive[_MAX_DRIVE], path[MAX_PATH];
	GetModuleFileNameA(AfxGetInstanceHandle(), filename, MAX_PATH);
	
	_splitpath(filename, drive, path, NULL, NULL);
	sprintf(appPath, "%s%s", drive, path);
}

static char iniFileName[256] = {0};
	
BOOL PluginApp::InitInstance()
{
	GetModuleFileNameA(NULL,iniFileName, 250);
	char *p = strrchr(iniFileName, '.');
	*p = 0;
	strcat(iniFileName,".ini");
	
	HANDLE	hFile = CreateFile("QQwry.dat", 0, 0, NULL, OPEN_EXISTING, 0, NULL);
	if (hFile != INVALID_HANDLE_VALUE)
	{ 
		ISite::m_bIsQQwryExist = true;
		ISite::m_QQwry = new SEU_QQwry;
		ISite::m_QQwry->SetPath("QQWry.Dat");
	}
	else
	{
		ISite::m_bIsQQwryExist = false;
	}
	CloseHandle(hFile);

	return TRUE;
}

char* PluginApp::GetCFGFile(){return iniFileName;}


void PluginApp::OnPlugCmd(UINT id)
{
	PluginMgr::GetInstance()->OnCommand(id);
}

void PluginApp::OnPlugCmdUI(CCmdUI* pCmdUI)
{
	PluginMgr::GetInstance()->OnCommandUI(pCmdUI);
}

void PluginApp::LoadAllPlugins()
{
	char path[MAX_PATH];
	GetApplicationPath(path, MAX_PATH);
	strcat(path, "plugins\\");

	PluginMgr::GetInstance()->LoadAll(path);
}

void PluginApp::UnloadAllPlugins()
{
	PluginMgr::GetInstance()->FreeAll();
	PluginMgr::FreeInstance();
}

static void CreateDocTemplate(void* p, DllWrapper* wrapper)
{
	IPlugProxy& proxy = wrapper->GetPlugIn();
	if(proxy)
		proxy->OnCreateDocTemplate();
}

void PluginApp::RegisterAllDocTemplates()
{
	PluginMgr::GetInstance()->TraversalAll(CreateDocTemplate, NULL);
}

static DWORD dwMask=87654321;  //�����ӣ�����趨
static DWORD dwKey=0x20080808;  //��Կ�룬����Լ��üǡ����˲�֪��

static BOOL IsRegisterOK_()
{
	DWORD dwVolumeSerialNumber,dwUserID,dwDecodeNumber; 
	
	CString strUserID,strRegCode;
	char str[200];
	int str_len=200;

	// ��Ӳ�����к���������������Ϊ�û���
    GetVolumeInformation("C:\\",NULL,NULL,&dwVolumeSerialNumber,NULL,NULL,NULL,NULL);  
	dwUserID=dwVolumeSerialNumber^dwMask;  
	strUserID.Format("%d",dwUserID);

	// ��ȷ��ע����ӦΪ16������ʽ=(10���Ƶ�dwUserIDת16���ƺ�)^(16���Ƶ���Կ��)
	// �����߿����ÿ�ѧ�ͼ������������û���֪���û���ֱ�����ע���뷵�ظ��û�

    // ��ֱ�ӴӲ���ϵͳ��win.ini��ȡע����
	const char* soft = "CtrlWin7";
	const char* usrId = "UserID";
	const char* regCode = "RegCode";
	const char* cfgFile = PluginApp::GetCFGFile();

    if(GetPrivateProfileString(soft, regCode, "", str, str_len, cfgFile) != 0 )
    {
        strRegCode=str;
		
		// ��֤ע����
		dwDecodeNumber=strtoul(strRegCode,NULL,16);  
        dwDecodeNumber^=dwKey;  //����Կ
		dwDecodeNumber^=dwMask;  //��ԭӲ�����к�
		if(dwDecodeNumber==dwVolumeSerialNumber)  //ע��ɹ�
	    	return TRUE;
	}

    // ��������ȡ��ע���벻�Ի�û�У���ͨ���Ի���ȡע����
	for(;;)
	{
		CDlgRegister dlg;
		dlg.m_strUserID=strUserID;

		if(dlg.DoModal()==IDOK)
		{
			strRegCode=dlg.m_strRegCode;

			// ��֤ע����
			dwDecodeNumber=strtoul(strRegCode,NULL,16);  
            dwDecodeNumber^=dwKey;  //����Կ
			dwDecodeNumber^=dwMask;  //��ԭӲ�����к�

			if(dwDecodeNumber==dwVolumeSerialNumber)  //ע��ɹ�
			{
				// ��ͨ����ע����Ϣ���浽����ϵͳ��win.ini���Ա��Ժ�ֱ�Ӷ�ȡ
				WritePrivateProfileString(soft, usrId, strUserID, cfgFile);
				WritePrivateProfileString(soft, regCode, strRegCode, cfgFile);

	    		return TRUE;
			}
     		else
			    AfxMessageBox("ע������� ������������򿪷�����ȡ����ע���룡");
		}
		else
			break; 
	}

    return FALSE;  //ע��ʧ��
}

BOOL PluginApp::IsRegisterOK() 
{
	return IsRegisterOK_();    
}