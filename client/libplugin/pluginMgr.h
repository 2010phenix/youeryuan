// pluginMgr.h: interface for the pluginMgr class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PLUGINMGR_H__B7EC75A8_A9E7_4C17_9E10_98C5B9BC50D4__INCLUDED_)
#define AFX_PLUGINMGR_H__B7EC75A8_A9E7_4C17_9E10_98C5B9BC50D4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "iPlug.h"
#include <list>

// ��������࣬������Ҫ���������в�������������в��
class LIBPLUG_API PluginMgr  
{
public:
	virtual ~PluginMgr();

	// ����
	BOOL OnService(UINT notifyId, WPARAM wparam, LPARAM lparam);
	// ֪ͨ
	void OnNotify(UINT serviceId,WPARAM wparam,LPARAM lparam);
	
	//�����������Ӧ
	void OnCommand(UINT);
	void OnCommandUI(CCmdUI*);
	
	// �������в��
	void LoadAll(const char* path);

	// �ͷ����в��
	void FreeAll();
	
	//�������в��
	void TraversalAll(void (*f)(void*, DllWrapper*), void* param);
	
	// �õ�����ĸ���
	int	GetCount();

	//	�õ�ʵ��
	static PluginMgr* GetInstance();
	// �ͷ�ʵ��
	static void FreeInstance();

private:
	PluginMgr();
	
	// �ļ��б�
	std::list<DllWrapper*> m_dllList;

	friend class PluginMainFrame;
};

#endif // !defined(AFX_PLUGINMGR_H__B7EC75A8_A9E7_4C17_9E10_98C5B9BC50D4__INCLUDED_)
