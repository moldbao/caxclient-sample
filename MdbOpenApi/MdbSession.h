#pragma once

#include "stdafx.h"

#include "MdbConstants/MdbConstants.h"
#include "MdbHttpHelper.h"
#include "MdbCommon.h"
#include "ZipHelper.h"

class MDBAPI MdbSession
{
public:
	~MdbSession(void);

	static MdbSession& getSession(const char *userName="",const char * password="");

	//	��ʼ�� log �� ����ָ��
	static void initLogFun(LogFun _pLog,LogFun _pStatus,LogFun _pEcho,LogFun _pExcept,MsgFun _pMsg);

	//	��ȡ CAM ����
	vector<pWebCAMTask> getCAMTasks();

	//	���ص���
	string downloadCamTaskFile(pWebCAMTask myTask,string &saveFolder);

	//	�ϴ� NC ��Ϣ
	map<string,string> uploadNCInfo(string &para);

	//	�ϴ� NC �ļ�
	bool uploadNCFile(pWebCAMTask myTask,string sCamGroupId,string sProgramTypeId,string fileType,string filePath,string sparkType="",string fileNames="",string uploadFileType="1000");

	//	�����깤
	bool finishTask(pWebCAMTask myTask);

	//	��ȡ ��ʽ����
	vector<pCncTemplate> getNCTempaltes(pWebCAMTask myTask,string sectionType="CNC");

	bool isTest;	//	�Ƿ� Ϊ ���Ի���

	//	���� partId ��������ĸ������嵥
	map<string,string> getPartFiles(string partId);
private:

	MdbSession(const char *userName="",const char * password="");
	MdbSession(const MdbSession&){return;}
	MdbSession& operator=(const MdbSession&a){return getSession();}

	MdbConstants* mdbConstants;		//	�����ļ�����

	MdbHttpHelper* http;			//	http����
};

