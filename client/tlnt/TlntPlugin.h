// TlntPlugin.h: interface for the TlntPlugin class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TLNTPLUGIN_H__251A997E_54F1_44A7_837A_FAF70E7A6A2F__INCLUDED_)
#define AFX_TLNTPLUGIN_H__251A997E_54F1_44A7_837A_FAF70E7A6A2F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "iPlug.h"

class TlntPlugin : public IPlugIn  
{
public:
	TlntPlugin();
	virtual ~TlntPlugin();

	// �����ʼ��
	virtual BOOL	OnInitalize();
	// ж�ز��
	virtual BOOL	OnUninitalize();
	
	// ************************************
	// �����ĵ�ģ��
	virtual void OnCreateDocTemplate();

	// ������ܲ˵�
	virtual void OnCreateFrameMenu(CMenu* pMainMenu);

	// ������ܹ�����
	virtual	void OnCreateFrameToolBar(ToolBarData*,UINT& count);

	// ����ͣ������
	virtual void OnCreateDockPane(DockPane* ,UINT& count);

	// �˵�������������
	virtual void	OnCommand(UINT resId);
	virtual void	OnCommandUI(CCmdUI* pCmdUI);
	// �յ�֪ͨ��֪ͨ�ţ�����1������2
	virtual void	OnNotify(UINT,WPARAM,LPARAM);

private:
	CMultiDocTemplate* m_docTemp;
};

#endif // !defined(AFX_TLNTPLUGIN_H__251A997E_54F1_44A7_837A_FAF70E7A6A2F__INCLUDED_)
