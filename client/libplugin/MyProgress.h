#if !defined(AFX_MYPROGRESS_H__F0A27541_9BA2_4C06_9863_2DF412B6BCDC__INCLUDED_)
#define AFX_MYPROGRESS_H__F0A27541_9BA2_4C06_9863_2DF412B6BCDC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MyProgress.h : header file
//

#include "resource.h"

/////////////////////////////////////////////////////////////////////////////
// MyProgress dialog

class MyProgress : public CDialog
{
// Construction
public:
	enum TRANSTYPE{UPLOAD, DOWNLOAD};
	MyProgress(enum TRANSTYPE type, CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(MyProgress)
	enum { IDD = IDD_PROGRESS };
	CProgressCtrl	m_progress;
	CStatic	m_fromS;
	CStatic	m_nameS;
	CStatic	m_remainS;
	CStatic	m_speedS;
	CStatic	m_toS;
	//}}AFX_DATA

	//���ô�������
	void SetTransferType(enum TRANSTYPE type);
	
	//���ô����ļ�
	void SetFile(const char* name);

	//���ô���Ŀ���ַ��Դ��ַ
	void SetSource(const char* src);
	void SetTarget(const char* tgt);

	void SetSubdir(const char* subDir);

	//�����ļ���С���Ѿ�����Ĵ�С�Լ������ٶ�
	void SetElapsedBytes(__int64 rSize);
	void SetTotalBytes(__int64 tSize);
	void SetSpeed(__int64 speed, int isTime = 0);


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(MyProgress)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(MyProgress)
		// NOTE: the ClassWizard will add member functions here
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	virtual void PostNcDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()


	TRANSTYPE m_type;
	CString	m_from2;
	CString	m_to2;
	__int64 totalSize;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MYPROGRESS_H__F0A27541_9BA2_4C06_9863_2DF412B6BCDC__INCLUDED_)
