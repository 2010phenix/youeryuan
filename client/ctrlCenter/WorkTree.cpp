// WorkTree.cpp : implementation file
//

#include "stdafx.h"
#include "WorkTree.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define   DRAG_DELAY   60

/////////////////////////////////////////////////////////////////////////////
// CWorkTree

CWorkTree::CWorkTree()
{
	m_bDragging = false;
	m_TimerTicks = 0;
	m_nHoverTimerID = 0;
	m_nScrollTimerID = 0;
	memset(m_draginfo, 0, sizeof(m_draginfo));
}

CWorkTree::~CWorkTree()
{
}


BEGIN_MESSAGE_MAP(CWorkTree, CTreeCtrl)
	//{{AFX_MSG_MAP(CWorkTree)
	ON_NOTIFY_REFLECT(TVN_BEGINDRAG, OnBegindrag)
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDOWN()
	ON_WM_TIMER()
	ON_WM_RBUTTONDOWN()
	ON_NOTIFY_REFLECT(TVN_BEGINLABELEDIT, OnBeginlabeledit)
	//ON_NOTIFY_REFLECT(TVN_ENDLABELEDIT, OnEndlabeledit)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWorkTree message handlers

void CWorkTree::OnBegindrag(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	*pResult = 0;

	//�����������ҷ�����������
	if( (GetTickCount() - m_dwDragStart) < DRAG_DELAY )
		return;	
	m_hItemDragS = pNMTreeView->itemNew.hItem;
	m_hItemDragD = NULL;

	//��������϶�Ŀ����������
	if(GetItemData(m_hItemDragS) < 2000) return;

	//�õ������϶�ʱ��ʾ��ͼ���б�
	m_pDragImage = CreateDragImage( m_hItemDragS );
	if( !m_pDragImage )
		return;

	m_bDragging = true;
	m_pDragImage->BeginDrag ( 0,CPoint(8,8) );
	CPoint  pt = pNMTreeView->ptDrag;
	ClientToScreen( &pt );
	m_pDragImage->DragEnter ( this,pt );  //"this"����ҷ���������ڸô���
	SetCapture();

	m_nScrollTimerID = SetTimer( 2,40,NULL );
}

void CWorkTree::OnMouseMove(UINT nFlags, CPoint point)
{
	HTREEITEM  hItem;
	UINT       flags;

	//���������ж�ʱ���Ƿ����,���������ɾ��,ɾ�����ٶ�ʱ
	if( m_nHoverTimerID )
	{
		KillTimer( m_nHoverTimerID );
		m_nHoverTimerID = 0;
	}
	m_nHoverTimerID = SetTimer( 1,800,NULL );  //��ʱΪ 0.8 �����Զ�չ��
	m_HoverPoint = point;

	if( m_bDragging )
	{
		CPoint  pt = point;
		CImageList::DragMove( pt );

		//��꾭��ʱ������ʾ
		CImageList::DragShowNolock( false );  //������꾭��ʱ�����ѿ��ĺۼ�
		if( (hItem = HitTest(point,&flags)) != NULL )
		{
			SelectDropTarget( hItem );
			m_hItemDragD = hItem;
		}
		CImageList::DragShowNolock( true );

	}

	CTreeCtrl::OnMouseMove(nFlags, point);
}

void CWorkTree::OnLButtonDown(UINT nFlags, CPoint point)
{
	//����������ҷ
	m_dwDragStart = GetTickCount();

	CTreeCtrl::OnLButtonDown(nFlags, point);
}

void CWorkTree::OnLButtonUp(UINT nFlags, CPoint point)
{
	CTreeCtrl::OnLButtonUp(nFlags, point);

	if( m_bDragging )
	{
		m_bDragging = FALSE;
		CImageList::DragLeave( this );
		CImageList::EndDrag();
		ReleaseCapture();
		delete m_pDragImage;

		SelectDropTarget( NULL );
		
		if( m_hItemDragS == m_hItemDragD )
		{
			KillTimer( m_nScrollTimerID );
			return;
		}

		Expand( m_hItemDragD,TVE_EXPAND );

		HTREEITEM  htiParent = m_hItemDragD;

		//Ŀ���������ΪĿ¼
		if(m_hItemDragD == NULL 
			|| (GetItemData( m_hItemDragD ) >= FIRST_OBJ_DATA) 
		  )
//			|| (GetItemData( m_hItemDragD ) == ROOT_DATA) )
		{
			SelectItem(m_hItemDragS);
			KillTimer(m_nScrollTimerID);
			return;
		}
	
		while( (htiParent = GetParentItem(htiParent)) != NULL )
		{
			if( htiParent == m_hItemDragS )
			{
				HTREEITEM  htiNewTemp = CopyBranch( m_hItemDragS,NULL,TVI_LAST );
				HTREEITEM  htiNew = CopyBranch( htiNewTemp,m_hItemDragD,TVI_LAST );
				DeleteItem( htiNewTemp );
				SelectItem( htiNew );
				KillTimer( m_nScrollTimerID );
				return;
			}
		}

		HTREEITEM  htiNew = CopyBranch( m_hItemDragS,m_hItemDragD,TVI_LAST );
		DeleteItem( m_hItemDragS );
		SelectItem( htiNew );
		KillTimer( m_nScrollTimerID );
		SendMsgForDragFinished(WM_DRAG_OBJ_FINISHED);
	}
}

void CWorkTree::OnTimer(UINT nIDEvent)
{
	//������нڵ�
	if( nIDEvent == m_nHoverTimerID )
	{
		KillTimer( m_nHoverTimerID );
		m_nHoverTimerID = 0;
		HTREEITEM  trItem = 0;
		UINT  uFlag = 0;
		trItem = HitTest( m_HoverPoint,&uFlag );
		if( trItem && m_bDragging )
		{
			SelectItem( trItem );
			Expand( trItem,TVE_EXPAND );
		}
	}
	//������ҷ�����еĹ�������
	else if( nIDEvent == m_nScrollTimerID )
	{
		m_TimerTicks++;
		CPoint  pt;
		GetCursorPos( &pt );
		CRect  rect;
		GetClientRect( &rect );
		ClientToScreen( &rect );

		HTREEITEM  hItem = GetFirstVisibleItem();
		
		if( pt.y < rect.top +10 )
		{
			//���Ϲ���
			int  slowscroll = 6 - (rect.top + 10 - pt.y )/20;
			if( 0 == (m_TimerTicks % ((slowscroll > 0) ? slowscroll : 1)) )
			{
				CImageList::DragShowNolock ( false );
				SendMessage( WM_VSCROLL,SB_LINEUP );
				SelectDropTarget( hItem );
				m_hItemDragD = hItem;
				CImageList::DragShowNolock ( true );
			}
		}
		else if( pt.y > rect.bottom - 10 )
		{
			//���¹���
			int  slowscroll = 6 - (pt.y - rect.bottom + 10)/20;
			if( 0 == (m_TimerTicks % ((slowscroll > 0) ? slowscroll : 1)) )
			{
				CImageList::DragShowNolock ( false );
				SendMessage( WM_VSCROLL,SB_LINEDOWN );
				int  nCount = GetVisibleCount();
				for( int i=0 ; i<nCount-1 ; i++ )
					hItem = GetNextVisibleItem( hItem );
				if( hItem )
					SelectDropTarget( hItem );
				m_hItemDragD = hItem;
				CImageList::DragShowNolock ( true );
			}
		}
	}
	else
		CTreeCtrl::OnTimer(nIDEvent);
}

HTREEITEM CWorkTree::CopyBranch(HTREEITEM htiBranch, HTREEITEM htiNewParent, HTREEITEM htiAfter)
{
	HTREEITEM  hChild;
	HTREEITEM  hNewItem = CopyItem( htiBranch,htiNewParent,htiAfter );
	hChild = GetChildItem( htiBranch );

	while( hChild != NULL )
	{
		CopyBranch( hChild,hNewItem,htiAfter );
		hChild = GetNextSiblingItem( hChild );
	}

	return  hNewItem;
}

HTREEITEM CWorkTree::CopyItem(HTREEITEM hItem, HTREEITEM htiNewParent, HTREEITEM htiAfter)
{
	TV_INSERTSTRUCT  tvstruct;
	HTREEITEM        hNewItem;
	CString          sText;

	//�õ�Դ��Ŀ����Ϣ
	tvstruct.item.hItem = hItem;
	tvstruct.item.mask  = TVIF_CHILDREN|TVIF_HANDLE|TVIF_IMAGE|TVIF_SELECTEDIMAGE;
	GetItem( &tvstruct.item );
	sText = GetItemText( hItem );
	tvstruct.item.cchTextMax = sText.GetLength ();
	tvstruct.item.pszText    = sText.LockBuffer ();

	//����Ŀ���뵽���ʵ�λ��
	tvstruct.hParent         = htiNewParent;
	tvstruct.hInsertAfter    = htiAfter;
	tvstruct.item.mask       = TVIF_IMAGE|TVIF_SELECTEDIMAGE|TVIF_TEXT;
	hNewItem = InsertItem( &tvstruct );
	sText.ReleaseBuffer ();

	//���ƿ�����Ŀ���ݺ���Ŀ״̬
	SetItemData( hNewItem,GetItemData(hItem) );
	SetItemState( hNewItem,GetItemState(hItem,TVIS_STATEIMAGEMASK),TVIS_STATEIMAGEMASK);

	return hNewItem;
}


/***************************************************************
*
* ���ƣ�FindItem
*
* ���ߣ�syq
*
* ���ܣ������ؼ��и����������ֵ�������ڵ�,�������
*
* ������item    - Ҫ��ѯ�����ڵ㼰���ӽڵ�
*
*       int		- Ҫ��ѯ���������ֵ
*
* ����ֵ��NULL  - û�ҵ�
*      
*        (�ǿյ�HTREEITEM ) - �ҵ�
*
***************************************************************/
HTREEITEM CWorkTree::FindItem(HTREEITEM item, unsigned int i)
{
	HTREEITEM hFind;	

	if( item == NULL )
		return NULL;

	while(item != NULL)
	{
		if( GetItemData(item) == i)
			return item;

		if( ItemHasChildren(item))
		{
			item = GetChildItem(item);
			hFind = FindItem(item,i);		
			if(hFind)	return hFind;
			else item= GetNextSiblingItem(GetParentItem(item));
		}
		else
		{
			item = GetNextSiblingItem(item);
			if( item==NULL )	return NULL;
		}
	}

	return item;
}

void CWorkTree::OnRButtonDown(UINT nFlags, CPoint point) 
{
	HTREEITEM hItem = HitTest(point, &nFlags);
	if(hItem)
		SelectItem(hItem);
	CTreeCtrl::OnRButtonDown(nFlags, point);
}

void CWorkTree::OnBeginlabeledit(NMHDR* pNMHDR, LRESULT* pResult) 
{
	TV_DISPINFO* pTVDispInfo = (TV_DISPINFO*)pNMHDR;
	// TODO: Add your control notification handler code here
	if( pTVDispInfo->item.lParam == 0 )
		*pResult = 1;
	else
		*pResult = 0;	
	*pResult = 0;
}

/*
void CWorkTree::OnEndlabeledit(NMHDR* pNMHDR, LRESULT* pResult) 
{
	TV_DISPINFO* pTVDispInfo = (TV_DISPINFO*)pNMHDR;
	// TODO: Add your control notification handler code here
	int i = GetItemData(pTVDispInfo->item.hItem);
	if(pTVDispInfo->item.pszText != NULL && i>=100 && i<2000)
		SetItemText( pTVDispInfo->item.hItem, pTVDispInfo->item.pszText );
	*pResult = 0;
}
*/


LRESULT CWorkTree::SendMsgForDragFinished( int message )
{
	if(!IsWindow(m_hWnd))
		return 0;
	CWnd *pOwner = GetOwner();
    if (pOwner && IsWindow(pOwner->m_hWnd))
	{
        return pOwner->PostMessage(message);
	}
	return 0;
	
}


