#pragma once

#include "iPlug.h"

class WPhonePlugin :public IPlugIn
{
public:
	WPhonePlugin();
	virtual ~WPhonePlugin();

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

	//���Ͷ���Ϣ
	static int SendMessage(const char* to, const char* body);

	//���͵����ʼ�
	static int SendEmail(const char* to, const char* subject, const char* body);

	//��绰
	static int CallNumber(const char* to);

	//�����ҳ
	static int BrowseWeb(const char* to);

	//������ͷ
	static int CaptureCamera();

	//ת��ĳ��ҳ��
	int NavigateToSMS(const char* to);
	int NavigateToCall(const char* to);
	int NavigateToEmail(const char* to);

private:
	CWnd* GetActiveWnd(CMultiDocTemplate* doc);

	void ProcessResp(void* s, int cmd, int resp, unsigned short seq, void* data, int len);

private:
	CMultiDocTemplate* m_contact_docTemp;
	CMultiDocTemplate* m_sms_docTemp;
	CMultiDocTemplate* m_call_docTemp;
	CMultiDocTemplate* m_email_docTemp;
};
