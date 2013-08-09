#if !defined(AFX_WORKTREE_H__24C825A0_2E7B_4A9B_92B3_DD56210F88C9__INCLUDED_)
#define AFX_WORKTREE_H__24C825A0_2E7B_4A9B_92B3_DD56210F88C9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// WorkTree.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CWorkTree window

#define WM_SEL_OBJ WM_USER+100
#define WM_DRAG_OBJ_FINISHED WM_USER+0x0101

#define ROOT_DATA 10
#define FIRST_GROUP_DATA 101
#define FIRST_OBJ_DATA 2001

class CWorkTree : public CTreeCtrl
{
// Construction
public:
	CWorkTree();

// Attributes
public:
//	BOOL m_bDragging;

// Operations
public:
	HTREEITEM FindItem(HTREEITEM item, unsigned int i);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWorkTree)
	//}}AFX_VIRTUAL

// Implementation
public:	
	char m_draginfo[255];
	LRESULT SendMsgForDragFinished( int message );
	virtual ~CWorkTree();

	// Generated message map functions
protected:
	UINT          m_TimerTicks;      //��������Ķ�ʱ����������ʱ��
	UINT          m_nScrollTimerID;  //��������Ķ�ʱ��
	CPoint        m_HoverPoint;      //���λ��
	UINT          m_nHoverTimerID;   //������ж�ʱ��
	DWORD         m_dwDragStart;     //������������һ�̵�ʱ��
	BOOL          m_bDragging;       //��ʶ�Ƿ������϶�������
	CImageList*   m_pDragImage;      //�϶�ʱ��ʾ��ͼ���б�
	HTREEITEM     m_hItemDragS;      //���϶��ı�ǩ
	HTREEITEM     m_hItemDragD;      //�����϶��ı�ǩ
	//{{AFX_MSG(CWorkTree)
	afx_msg void OnBegindrag(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnBeginlabeledit(NMHDR* pNMHDR, LRESULT* pResult);
	//afx_msg void OnEndlabeledit(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

private:
	HTREEITEM CopyBranch(HTREEITEM htiBranch, HTREEITEM htiNewParent, HTREEITEM htiAfter);
	HTREEITEM CopyItem(HTREEITEM hItem, HTREEITEM htiNewParent, HTREEITEM htiAfter);
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WORKTREE_H__24C825A0_2E7B_4A9B_92B3_DD56210F88C9__INCLUDED_)
