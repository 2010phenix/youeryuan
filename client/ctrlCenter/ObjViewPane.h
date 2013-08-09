// ObjViewPane.h: interface for the CObjViewPane class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_OBJVIEWPANE_H__2BC263B0_ED42_4E74_9203_B7151D1E5016__INCLUDED_)
#define AFX_OBJVIEWPANE_H__2BC263B0_ED42_4E74_9203_B7151D1E5016__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "DockPaneWnd.h"
#include "WorkTree.h"

class SEU_QQwry;

//���ͽṹ��Ҷ�ӽ�����ʼֵ���Ժ����ӽ��ֵ����
#define		MIN_LEAF_NODE	2000

enum Status{ONLINE, OFFLINE, DESCRIPTION, NOP};
class CObjViewPane : public CWnd  
{
// Construction
public:
	CObjViewPane();
	virtual ~CObjViewPane();


// Attributes
public:
	BOOL	CreateImageList(CImageList& il, UINT nID);

	//�����µ����νڵ�
	HTREEITEM InsertItem(STANDARDDATA* info);
	
	//�ı����νڵ������
	int ChangeText(STANDARDDATA* info);

	//ɾ��һ�����νڵ� 
	int RemoveItem(STANDARDDATA* info);

	bool RemoveItem(HTREEITEM item);

	//��ȡ�����ѡ�е�Ҷ�ӽ���ֵ
	int GetValidObjData();

	int FindGroup();

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSelchangedTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRclickWorkTree(NMHDR* pNMHDR, LRESULT* pResult);
	
	//��ȡĿ�������
	afx_msg void OnGetObjMachineInfo();
	
	//�ػ�
	afx_msg void OnShutdownObjMachine();

	//����
	afx_msg void OnRebootObjMachine();

	//������
	afx_msg void OnRenameObjMachine();

	//�������еĲ����������
	afx_msg void OnUnloadPlugin();

	//ж�ط����������в��
	afx_msg void OnUpdatePlugin();

	//ж�ط�����
	afx_msg void OnRemoveService();

	DECLARE_MESSAGE_MAP()

private:
	int		m_iTempObj;
	HANDLE	m_hSelfDeleteThread;		
	unsigned int m_dwSelfDeleteThreadId;			
	CFont m_Font;
	CDockPaneWnd<CWorkTree> m_wndObjView;
	CImageList m_ilClassView;
	HTREEITEM	m_hItemNew;		//��ǰѡ�е����ͽ��
	HTREEITEM	m_hItemOld;		//�ղ�ѡ�е����ͽ��
	int m_iObjData;
	int m_iGroupData;
	HTREEITEM m_root;		//Ŀ������ͽṹ���ڵ���
	HTREEITEM m_hti;
};

#endif // !defined(AFX_OBJVIEWPANE_H__2BC263B0_ED42_4E74_9203_B7151D1E5016__INCLUDED_)
