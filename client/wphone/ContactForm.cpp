#include "StdAfx.h"
#include "ContactForm.h"
#include "WPhonePlugin.h"

#define ID_MENU_EXPORT	10010
#define ID_MENU_CALL	10011
#define ID_MENU_SMS		10012
#define ID_MENU_EMAIL	10013

#define ID_TREE_LIST_HEADER 3270
#define ID_TREE_LIST_CTRL   3273
#define ID_TREE_LIST_SCROLLBAR   3277

IMPLEMENT_DYNCREATE(ContactForm, CFormView)

ContactForm::ContactForm(): CFormView(ContactForm::IDD)
{
	m_init = FALSE;
}

ContactForm::~ContactForm()
{
}

void ContactForm::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(ContactForm, CFormView)
	//{{AFX_MSG_MAP(CSysInfoForm)
	ON_WM_CREATE()
	ON_COMMAND(ID_MENU_EXPORT,OnMenuExport)
	ON_COMMAND(ID_MENU_CALL, OnCallContact)
	ON_COMMAND(ID_MENU_SMS, OnSMSContact)
	ON_COMMAND(ID_MENU_EMAIL, OnEmailContact)	
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void ContactForm::OnSize(UINT nType, int cx, int cy) 
{
	CFormView::OnSize(nType, cx, cy);
	
	if(m_init)
	{
		CRect m_wndRect;
		GetClientRect(&m_wndRect);

		CRect m_headerRect;
		m_treeList.m_wndHeader.GetClientRect(&m_headerRect);
		m_treeList.m_wndHeader.SetWindowPos(&wndTop, 0, 0, m_wndRect.Width(), m_headerRect.Height(), SWP_NOMOVE);

		m_treeList.SetWindowPos(&wndTop, 0, 0, m_wndRect.Width(), m_wndRect.Height(), SWP_NOMOVE);
	}
}

void ContactForm::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();
	GetParentFrame()->RecalcLayout();
	ResizeParentToFit();

	CRect m_wndRect;
	GetClientRect(&m_wndRect);
	CRect m_headerRect;


	// create the header
	{
		m_headerRect.left = m_headerRect.top = -1;
		m_headerRect.right = m_wndRect.Width();

		m_treeList.m_wndHeader.Create(WS_CHILD | WS_VISIBLE | HDS_BUTTONS | HDS_HORZ, m_headerRect, this, ID_TREE_LIST_HEADER);
	}

	static CFont font;
	font.CreateFont(100, 0, 0, 0, FW_NORMAL, FALSE,FALSE,FALSE,GB2312_CHARSET,
		OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,
		DEFAULT_PITCH|FF_MODERN,"宋体");
	//m_treeList.m_wndHeader.SetFont(&font);
	m_treeList.SetFont(&font);

	CSize textSize;
	// set header's pos, dimensions and image list
	{
		CDC *pDC = m_treeList.m_wndHeader.GetDC();
		pDC->SelectObject(&m_treeList.m_headerFont);
		textSize = pDC->GetTextExtent("A");

		m_treeList.m_wndHeader.SetWindowPos(&wndTop, 0, 0, m_headerRect.Width(), textSize.cy+4, SWP_SHOWWINDOW);
		m_treeList.m_wndHeader.UpdateWindow();
	}

	CRect m_treeListRect;
	// create the tree itself
	{
		GetClientRect(&m_wndRect);

		m_treeListRect.left=0;
		m_treeListRect.top = textSize.cy+4;
		m_treeListRect.right = m_headerRect.Width()-5;
		m_treeListRect.bottom = m_wndRect.Height()-GetSystemMetrics(SM_CYHSCROLL)-4;

		m_treeList.Create(WS_CHILD | WS_VISIBLE | TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS | TVS_SHOWSELALWAYS, m_treeListRect, this, ID_TREE_LIST_CTRL);
	}

	m_treeList.InsertColumn(0, "姓名", LVCFMT_LEFT, 200);
	m_treeList.InsertColumn(1, "电话", LVCFMT_LEFT,  200);
	m_treeList.InsertColumn(2, "地址", LVCFMT_LEFT, 200);
	m_treeList.InsertColumn(3, "Email", LVCFMT_LEFT,  200);
	ClearAll();

	m_init = TRUE;
	//SortTree(0, TRUE, m_treeList.GetRootItem());
}
int	ContactForm::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	return CFormView::OnCreate(lpCreateStruct);
}

void ContactForm::ClearAll()
{
	m_treeList.DeleteAllItems();
}

void ContactForm::Insert(unsigned short seq, void* data)
{
	char* p = (char*)data;

	CString name = p;
	p += strlen(p) + 1; 

	CString phone = p;
	p += strlen(p) + 1; 

	CString addr = p;
	p += strlen(p) + 1; 
	
	CString email = p;

	HTREEITEM hTmp = m_treeList.InsertItem(name, 0, 0, NULL);
	
	if(hTmp == NULL)	return;

	m_treeList.SetItemText(hTmp, 1, phone);
	m_treeList.SetItemText(hTmp, 2, addr);
	m_treeList.SetItemText(hTmp, 3, email);
}

BOOL ContactForm::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult) 
{
	HD_NOTIFY *pHDN = (HD_NOTIFY*)lParam;	

	if((wParam == ID_TREE_LIST_HEADER) && (pHDN->hdr.code == HDN_ITEMCLICK))
	{
		int nCol = pHDN->iItem;

		BOOL bAscending = FALSE;

		if(m_treeList.m_wndHeader.GetItemImage(nCol)==-1)
			bAscending = TRUE;
		else
			if(m_treeList.m_wndHeader.GetItemImage(nCol)==1)
				bAscending = TRUE;

		for(int i=0;i<m_treeList.GetColumnsNum();i++)
		{
			m_treeList.m_wndHeader.SetItemImage(i, -1);
		}

		if(bAscending)
			m_treeList.m_wndHeader.SetItemImage(nCol, 0);
		else
			m_treeList.m_wndHeader.SetItemImage(nCol, 1);

		m_treeList.SortItems(nCol, bAscending, NULL);
		SortTree(nCol, bAscending, m_treeList.GetRootItem());

		m_treeList.UpdateWindow();
	}
	else if((wParam == ID_TREE_LIST_HEADER) && (pHDN->hdr.code == HDN_ITEMCHANGED))
	{
		int m_nPrevColumnsWidth = m_treeList.GetColumnsWidth();
		m_treeList.RecalcColumnsWidth();
		//ResetScrollBar();

		// in case we were at the scroll bar's end,
		// and some column's width was reduced,
		// update header's position (move to the right).
		CRect m_treeRect;
		m_treeList.GetClientRect(&m_treeRect);

		CRect m_headerRect;
		m_treeList.m_wndHeader.GetClientRect(&m_headerRect);
		m_treeList.Invalidate();
	}
	else if(wParam == ID_TREE_LIST_CTRL && (pHDN->hdr.code == NM_RCLICK))
	{
		if(m_treeList.GetItemCount() > 0)
		{
			CMenu menu;
			menu.CreatePopupMenu();
			menu.AppendMenu(MF_STRING, ID_MENU_EXPORT, "导出");
			menu.AppendMenu(MF_STRING, ID_MENU_SMS, "发短信");
			menu.AppendMenu(MF_STRING, ID_MENU_CALL, "打电话");
			menu.AppendMenu(MF_STRING, ID_MENU_EMAIL, "发电子邮件");

			CPoint pt;
			GetCursorPos(&pt);
			int nCode = menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, this);
			menu.DestroyMenu();
		}
	}
	else
		GetParent()->SendMessage(WM_NOTIFY, wParam, lParam);

	return CFormView::OnNotify(wParam, lParam, pResult);
}

void ContactForm::SortTree(int nCol, BOOL bAscending, HTREEITEM hParent)
{
	HTREEITEM hChild;
	HTREEITEM hPa = hParent;
	while(hPa!=NULL)
	{
		if(m_treeList.ItemHasChildren(hPa))
		{
			hChild = m_treeList.GetChildItem(hPa);
			m_treeList.SortItems(nCol, bAscending, hPa);
			SortTree(nCol, bAscending, hChild);
		}

		hPa = m_treeList.GetNextSiblingItem(hPa);
	}
}

void ContactForm::OnMenuExport()
{
	CString szFileName;
	CFileDialog saveDialog(FALSE, "*.txt", "contact",  OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, "文本文件文件(*.txt)|*.txt||");
	if (saveDialog.DoModal() != IDOK) return;
	szFileName = saveDialog.GetPathName();

	CFile newfile;
	newfile.Open(szFileName,CFile::modeCreate | CFile::modeWrite);

	HTREEITEM hPa = m_treeList.GetRootItem();
	while(hPa!=NULL)
	{
		HTREEITEM next = m_treeList.GetNextSiblingItem(hPa); 
		CString str;

		for(int j = 0; j < 4; j++)
			str += m_treeList.GetItemText(hPa, j) + "\t";
		str += "\r\n";

		hPa = next;
		newfile.Write(str,str.GetLength());
	}
	newfile.Close();
}

void ContactForm::OnCallContact()
{
	HTREEITEM hPa = m_treeList.GetSelectedItem();
	if(hPa == NULL)	return;

	CString to =  m_treeList.GetItemText(hPa, 1);
	if(to == "")	return;

	gWPhonePlugin->NavigateToCall(to);
}
	

void ContactForm::OnSMSContact()
{
	HTREEITEM hPa = m_treeList.GetSelectedItem();
	if(hPa == NULL)	return;

	CString to =  m_treeList.GetItemText(hPa, 1);
	if(to == "")	return;

	gWPhonePlugin->NavigateToSMS(to);
}

void ContactForm::OnEmailContact()
{
	HTREEITEM hPa = m_treeList.GetSelectedItem();
	if(hPa == NULL)	return;

	CString to =  m_treeList.GetItemText(hPa, 3);
	if(to == "")	return;

	gWPhonePlugin->NavigateToEmail(to);
}
