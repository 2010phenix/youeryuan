// PluginApp.h: interface for the PluginApp class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PluginApp_H__B006C998_96CF_4429_85B7_934A393F58A7__INCLUDED_)
#define AFX_PluginApp_H__B006C998_96CF_4429_85B7_934A393F58A7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "iPlug.h"

class LIBPLUG_API PluginApp : public CWinApp  
{
public:
	PluginApp();
	virtual ~PluginApp();

	BOOL InitInstance();

	//�������п��õĲ��
	void LoadAllPlugins();

	//ж�ز��
	void UnloadAllPlugins();	

	//�������е��ĵ�ģ��
	void RegisterAllDocTemplates();
	
	//������¼�
	void OnPlugCmd(UINT id);
	
	//����˵�״̬����
	void OnPlugCmdUI(CCmdUI* pCmdUI);

	//ע�������
	BOOL IsRegisterOK();

	static char* GetCFGFile();

	DECLARE_MESSAGE_MAP()
};

#endif // !defined(AFX_PluginApp_H__B006C998_96CF_4429_85B7_934A393F58A7__INCLUDED_)
