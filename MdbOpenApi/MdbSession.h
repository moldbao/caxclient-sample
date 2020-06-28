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

	//	初始化 log 等 函数指针
	static void initLogFun(LogFun _pLog,LogFun _pStatus,LogFun _pEcho,LogFun _pExcept,MsgFun _pMsg);

	//	获取 CAM 任务
	vector<pWebCAMTask> getCAMTasks();

	//	下载档案
	string downloadCamTaskFile(pWebCAMTask myTask,string &saveFolder);

	//	上传 NC 信息
	map<string,string> uploadNCInfo(string &para);

	//	上传 NC 文件
	bool uploadNCFile(pWebCAMTask myTask,string sCamGroupId,string sProgramTypeId,string fileType,string filePath,string sparkType="",string fileNames="",string uploadFileType="1000");

	//	任务完工
	bool finishTask(pWebCAMTask myTask);

	//	获取 程式类型
	vector<pCncTemplate> getNCTempaltes(pWebCAMTask myTask,string sectionType="CNC");

	bool isTest;	//	是否 为 测试环境

	//	根据 partId 查找零件的附件的清单
	map<string,string> getPartFiles(string partId);
private:

	MdbSession(const char *userName="",const char * password="");
	MdbSession(const MdbSession&){return;}
	MdbSession& operator=(const MdbSession&a){return getSession();}

	MdbConstants* mdbConstants;		//	配置文件参数

	MdbHttpHelper* http;			//	http请求
};

