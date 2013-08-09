#pragma once

#include "iPlug.h"

class RemotePlugin : public IPlugIn
{
public:
	RemotePlugin();
	virtual ~RemotePlugin();

	// �����ʼ��
	virtual BOOL	OnInitalize();
	// ж�ز��
	virtual BOOL	OnUninitalize();

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
	virtual void OnNotify(UINT,WPARAM,LPARAM);

	//�ı�Ŀ¼������ĳ��Ŀ¼
	static void	OnRedirectDir(const char*);

	//�ļ�����
	int CreateUpDownFileThread(WPARAM);

	//ִ��ĳ���ļ�����
	static void OnFileOperation(unsigned short cmd, void* body, int len);
	static int DetachPathAndFile(const char* fullName, char* path, char* name);


private:
	CMultiDocTemplate* m_docTemp;
};
