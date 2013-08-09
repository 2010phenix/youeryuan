#pragma once

#include "iPlug.h"

class ProcessPlugin : public IPlugIn
{
public:
	ProcessPlugin();
	virtual ~ProcessPlugin();

	// �����ʼ��
	virtual BOOL OnInitalize();
	// ж�ز��
	virtual BOOL OnUninitalize();

	// �����ĵ�ģ��
	virtual void OnCreateDocTemplate();

	// ������ܲ˵�
	virtual void OnCreateFrameMenu(CMenu* pMainMenu);

	// ������ܹ�����
	virtual	void OnCreateFrameToolBar(ToolBarData*,UINT& count);

	// ����ͣ������
	virtual void OnCreateDockPane(DockPane* ,UINT& count);

	// �˵�������������
	virtual void OnCommand(UINT resId);
	virtual void OnCommandUI(CCmdUI* pCmdUI);

	// �յ�֪ͨ��֪ͨ�ţ�����1������2
	virtual void OnNotify(UINT, WPARAM, LPARAM);

	static void KillProcess(int pid);

private:
	CMultiDocTemplate* m_docTemp;
};
