#ifndef SVRCTRL2_H
#define SVRCTRL2_H

#include <vector>
#include <string.h>

/** 
 ���ݱ���������Ϣ������Ӧ��GUID
*/
int MakeGuid(unsigned char guidV[]);

/** 
 ���������������IP��ַ��Ϣ
*/
int GetSiteIP(std::vector<std::string>& ips, const char* hostname);


int RealService();

/**
 ������ǰ������Ȩ
*/
bool EnablePrivilege(const char* privilege, bool enable);

#endif