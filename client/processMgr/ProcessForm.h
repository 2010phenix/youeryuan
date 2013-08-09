#pragma once

#include <afxcview.h>
#include "afxcmn.h"
#include "resource.h"

// ProcessForm form view

class ProcessForm : public CFormView
{
	DECLARE_DYNCREATE(ProcessForm)

protected:
	ProcessForm();           // protected constructor used by dynamic creation
	virtual ~ProcessForm();

public:
	enum { IDD = IDD_FORMVIEW };
	
	//������Ӧ����
	void ProcessData(WPARAM wp);

#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif

protected:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnLvnColumnclickList1(NMHDR *pNMHDR, LRESULT *pResult);

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnInitialUpdate();

	//����Ҽ�ѡ��ĳ��item
	afx_msg void OnRclickItem(NMHDR* pNMHDR, LRESULT* pResult); 

	//ɱ������
	afx_msg void OnKillProcess();

public:
	//����ʧ��
	void OnOperationFailed(unsigned short cmd, unsigned short resp);

	//���½�����Ϣ
	void InsertData(void* body, int dataLen, int seq);
	
private:
	CListCtrl	m_listCtrl;

	DECLARE_MESSAGE_MAP()
};


