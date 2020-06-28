#include "MdbSession.h"

#include "MdbCommon.h"

#include "MdbGlogHelper.hxx"

#include "json/json.h"

void glogInit(const char *s/*="myLog"*/)
{
	GLogHelper::INIT(s);
}

MdbSession::MdbSession(const char *userName/*=""*/,const char * password/*=""*/)
{
	//printf("+++ create MdbSession\n");

	mdbConstants = new MdbConstants();
	MdbHttpHelper::mdbConstants = mdbConstants;

	string slogFolder = GetCurrentDir()+"\\glog";
	if (_access(slogFolder.c_str(),0)==-1)
		_mkdir(slogFolder.c_str());

	glogInit(("glog\\glog_"+GetCurrentDate()).c_str());  // 使用 glog 之前 必须 初始化 ... ...
	
	http = new MdbHttpHelper(userName,password);

	isTest = http->isTest();
}


MdbSession::~MdbSession(void)
{
	delete mdbConstants;
	delete http;

	//printf("--- create MdbSession\n");
}

MdbSession& MdbSession::getSession(const char *userName/*=""*/,const char * password/*=""*/)
{
	static MdbSession theSession(userName,password);
	return theSession;
}

void MdbSession::initLogFun( LogFun _pLog,LogFun _pStatus,LogFun _pEcho,LogFun _pExcept,MsgFun _pMsg )
{
	pLog = _pLog;
	pEcho = _pEcho;
	pExcept = _pExcept;
	pStatus = _pStatus;
	pMsg = _pMsg;
}

std::vector<pWebCAMTask> MdbSession::getCAMTasks()
{
	return http->getCAMTask();
}

std::string MdbSession::downloadCamTaskFile(pWebCAMTask myTask,string &saveFolder)
{
	string fileId = myTask->fileIdBomPart;

	if (fileId=="0") // 没有 PRT 档案的
	{
		map<string,string> mPartFiles = getPartFiles(myTask->partId);
		for each (pair<string,string> p in mPartFiles)
		{
			if (p.first.find(".x_t")!=string::npos || p.first.find(".X_T")!=string::npos ||
				p.first.find(".igs")!=string::npos || p.first.find(".IGS")!=string::npos ||
				p.first.find(".stp")!=string::npos || p.first.find(".STP")!=string::npos
				)
			{
				fileId = p.second;
			}
		}			
	}

	if (fileId=="0")
		EXCEPT("### 可能还没有上传附件！");

	string sout = "";
	if(http->downloadFile(fileId,saveFolder,sout))
		return sout;
	else
		EXCEPT("### 下载任务档案失败！");
}

map<string,string> MdbSession::uploadNCInfo(string &para)
{
	return http->uploadNCInfo(para);
}

bool MdbSession::uploadNCFile( pWebCAMTask myTask,string sCamGroupId,string sProgramTypeId,string fileType,string filePath,string sparkType/*=""*/,string fileNames/*=""*/,string uploadFileType/*="1000"*/)
{
	Json::Value juploadFile;

	juploadFile["camScheduleId"] = myTask->schMfCamId;
	juploadFile["bomPartId"] = myTask->partId;

	juploadFile["camGroupId_F"] = sCamGroupId;
	juploadFile["programTypeId"] = sProgramTypeId;//"683036359070021";
	juploadFile["fileType"] = fileType;//"PRT";

	juploadFile["uploadFileType"] = uploadFileType;//"1000";
	juploadFile["JSESSIONID"] = http->getJsessionId();//"2c1e94b9-7219-40b1-a43c-494b3578e1b2";

	if (sparkType.size()>0)
		juploadFile["gap"]=sparkType;

	if (fileNames.size()>0)
		juploadFile["fileNames"]=fileNames;

	return http->uploadFile(filePath,Json::FastWriter().write(juploadFile));
}

bool MdbSession::finishTask( pWebCAMTask myTask )
{
	return http->finishTask(myTask);
}

std::vector<pCncTemplate> MdbSession::getNCTempaltes(pWebCAMTask myTask,string sectionType/*="CNC"*/ )
{
	return http->getNCTempaltes(myTask,sectionType);
}

std::map<std::string,std::string> MdbSession::getPartFiles(string partId)
{
	return http->getPartFiles(partId);
}

