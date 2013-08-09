
#if !defined(AFX_CONSOLEEDIT_H__075B4737_E259_4C70_BCFF_920C70326027__INCLUDED_)
#define AFX_CONSOLEEDIT_H__075B4737_E259_4C70_BCFF_920C70326027__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ConsoleEdit.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CConsoleEdit window
const int SAVE_CMD_NUM = 5;
const int MAX_DATA_NUM = 24 * 1024;

typedef struct tagNM_CONSOLEEDIT {
    NMHDR hdr;
    char szText[255];
} NM_CONSOLEEDIT;

#define NM_CONSOLE_ENTER	WM_USER + 0x0101
#define NM_CONSOLE_CANCEL	WM_USER + 0x0102


class CConsoleEdit : public CEdit
{
// Construction
public:
	CConsoleEdit();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CConsoleEdit)
	public:
	virtual BOOL Create(const RECT& rect, CWnd* pParentWnd, UINT nID);
	//}}AFX_VIRTUAL

// Implementation
public:
	void OnMenuPlaster();
	void OnMenuCopy();
	void ClearText();
	bool GetCanInput();
	void SetWindowText(const char* str);
	void Refresh();
	// ����
	void Clear();
	// ��ֹ����
	void StopInput();
	// ��ʼ����
	void BeginInput();
	// ����һ���ַ�
	void AddTexts(char ch);
	// �����ַ���
	void AddTexts(const char* string, int len = -1);
	// �ƶ���ĩβ
	void MoveToEnd();
	virtual ~CConsoleEdit();

	// Generated message map functions
protected:
	//{{AFX_MSG(CConsoleEdit)
	afx_msg HBRUSH CtlColor(CDC* pDC, UINT nCtlColor);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnSetfocus();
	afx_msg void OnKillfocus();
	//}}AFX_MSG
private:
	// �õ���(��)һ������
	CString GetCommand(BOOL bUp = TRUE);
	// ����ԭ��������
	void SaveCommand(const char* cmd);
	// �õ�����λ��
	long GetCurrentPosition();
	// ������Ϣ��������
	LRESULT SendMessageToParent(const char*, int message);
	// �滻����
	void ReplaceCommand(const char* cmd);
public:
	bool m_bcopy;
private:
	COLORREF	m_clrText;
	COLORREF	m_clrBkgnd;
	CBrush		m_brBkgnd;
	CString		m_pretitle;					// ��ʾ�������C:\WINNT
	long		m_nLength;					// �����ܳ���
	char		m_cmd[255];					// ����
	CFont		m_font;
	bool		m_ischinese;				// Ŀǰ������Ƿ��Ǻ���
	CString		m_save_cmd[SAVE_CMD_NUM];	// ������������
	int			m_save_index;				// ��ȡ�������������	
	long		m_last_title_pos;			// ���һ����ʾ����λ��
	bool		m_can_input;				// ��������ı��


	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CONSOLEEDIT_H__075B4737_E259_4C70_BCFF_920C70326027__INCLUDED_)
