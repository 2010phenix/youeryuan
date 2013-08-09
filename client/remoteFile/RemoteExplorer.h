#pragma once

#include "BtnST.h"
#include <afxcview.h>
#include "afxcmn.h"
#include <list>
#include <vector>

// RemoteExplorer form view

class RemoteExplorer : public CFormView
{
	DECLARE_DYNCREATE(RemoteExplorer)

protected:
	RemoteExplorer();           // protected constructor used by dynamic creation
	virtual ~RemoteExplorer();

public:
	enum { IDD = IDD_FORMVIEW1 };
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif

protected:
	afx_msg void OnSize(UINT nType, int cx, int cy);

	//���˫��ĳ��item
	afx_msg void OnDblclkItem(NMHDR* pNMHDR, LRESULT* pResult);
	//����Ҽ�ѡ��ĳ��item
	afx_msg void OnRclickItem(NMHDR* pNMHDR, LRESULT* pResult); 
	//���˵���һ����Ŀ¼
	afx_msg void OnUpDir();
	//ˢ�µ�ǰĿ¼
	afx_msg void OnRefreshDir();
	//�ı��������·������
	afx_msg void OnSelChangeComBox();

	//��Ŀ¼
	afx_msg void OnOpenDir();
	//����Ŀ¼
	afx_msg void OnCreateDir();
	//����
	afx_msg void OnSearchFile();
	//������
	afx_msg void OnRenameFile();
	//ɾ��
	afx_msg void OnDeleteFile();
	//Զ������
	afx_msg void OnExecFile();
	//����
	afx_msg void OnCopyFile();
	//ճ��
	afx_msg void OnPasteFile();
	//����
	afx_msg void OnDownloadFile();
	//�ϴ�
	afx_msg void OnUploadFile();
	//�ϴ�
	afx_msg void OnUploadDir();

	typedef struct ICONINFO
	{
		char ext[5];
		int index;
	}ICONINFO;

public:
	//���ITEM�б�
	bool InsertItem(WPARAM wp, LPARAM lp);

	//�ļ������ɹ�
	void OnFileOperationCompleted(unsigned short cmd);
	//�ļ�����ʧ��
	void OnFileOperationFailed(unsigned short cmd, unsigned short resp);

private:
	//����LIST��ͷ
	void SetColumn(bool isDisk);
	//���뵥��������
	bool InsertDrive(void* p);
	//���뵥���ļ�/Ŀ¼
	bool InsertFileOrDir(void* p);
	//Ѱ�Һ��ʵ��ļ�ͼ��
	void FindImageIconId(LPCTSTR ItemName,int &nImage);
	//��ǰѡ����ļ�
	CString GetSelectedFile();
	//ִ�д�Ŀ¼�����ļ�����
	void OpenDirOrExecuteFile();

private:
	CButtonST	m_btnUpDir;
	CButtonST	m_btnRefresh;
	CButtonST	m_btnSearch;
	CComboBox	m_comDir;
	CListCtrl	m_listCtrl;
	CImageList	m_imgList;
	std::list<ICONINFO> m_list;
	std::vector<CString>	m_Drives;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnInitialUpdate();
	virtual BOOL PreTranslateMessage(MSG* pMsg); 

	DECLARE_MESSAGE_MAP()
};


