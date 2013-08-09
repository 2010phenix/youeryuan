#pragma once

#include "iPlug.h"

class AudioPlugin : public IPlugIn
{
public:
	AudioPlugin();
	virtual ~AudioPlugin();

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

	static void CloseAudio();

	static unsigned WINAPI audioThread(void* p);

private:
	CMultiDocTemplate* m_docTemp;
};
