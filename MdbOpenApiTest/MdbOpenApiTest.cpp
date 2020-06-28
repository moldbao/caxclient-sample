// MdbOpenApiTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <MdbSession.h>
#include "Json/json.h"

#include "MdbGlogHelper.hxx"
void showLog(char *s){	GINFO << s;}
void showExcept(char *s){	GERROR << s;}

const string userName = "";					//	用户名	| 为空时 从配置文件中加载
const string password = "";					//	密码		| 为空时 从配置文件中加载
const string moldNo = "CS2020";				//	目标 制品号
const string partNo = "C006";				//	目标 零件编号
const string targetLocalFolder = "D:\\";	//	本地保存目录
const string cncPostName = "FANUC_NC";		//	后处理类型
const bool doFinishCAMTask = false;			//	是否需要CAM任务自动完工

string cCurrentFolder = "";

//	获取 NC 程式的详细数据信息 | 此处为演示代码，从json文件中加载，实际需从CAX软件中获取
Json::Value getNCInfo(bool isTest)
{
	string sNCJson = cCurrentFolder + (isTest ? "\\data\\CS2020-C006-CNC-T_test.json": "\\data\\CS2020-C006-CNC-T.json");

	bool bResp = true;
	Json::Value root;
	Json::Reader reader;
	std::ifstream ifs(sNCJson.c_str());
	string sErrMsg = "";
	if(!reader.parse(ifs, root))
	{
		bResp = false;
		sErrMsg = "### Json parse Error : " + reader.getFormattedErrorMessages();
	}

	if (ifs.is_open())
		ifs.close();

	if (!bResp)
		EXCEPT("%s",sErrMsg.c_str());

	return root;
}

//	上传 NC 数据
bool uploadNCData(MdbSession& theSession,string &groupId)
{
	try
	{
		Json::Value jNCInfo = getNCInfo(theSession.isTest);
		if (jNCInfo.empty())
			EXCEPT("### 获取NC信息失败！");

		string sResult="";
		map<string,string> mNCIDs = theSession.uploadNCInfo(Json::FastWriter().write(jNCInfo));
		if (mNCIDs.size()!=1)
			EXCEPT("### NC数据上传失败 ： mNCIDs.size() = %d",mNCIDs.size());

		groupId = mNCIDs.begin()->second;
		return true;
	}
	catch (exception &e)
	{
		EXCEPT("### 上传NC数据失败 ： %s",e.what());
	}
	return false;
}

//	创建 压缩包 并 上传
void uploadFile(MdbSession& theSession,string &sGroupId,pWebCAMTask myTask,vector<string> &vFiles,string sZip,string sProgramTypeId,string fileType,string sparkType="",string fileNames="")
{
	string sPartShortName = sZip.substr(sZip.rfind('\\')+1);

	ECHO(">>> 正在压缩 %s ...",sPartShortName.c_str());
	if(!ZipFiles(vFiles,sZip))
		EXCEPT("### 创建压缩包 %s 失败！",sPartShortName.c_str());

	ECHO(">>> 正在上传 %s ...",sPartShortName.c_str());
	if(!theSession.uploadNCFile(myTask,sGroupId,sProgramTypeId,fileType,sZip,sparkType,fileNames))
		EXCEPT("### 上传文件 %s 失败！",sPartShortName.c_str());
}

//	上传 NC 文件
void uploadNCFile(MdbSession& theSession,string &sGroupId,pWebCAMTask myTask,string cncTemplateId)
{
	//	创建档案 PRT 压缩包
	vector<string> vParts;
	vParts.push_back(cCurrentFolder+"\\data\\CS2020-C006-CNC-T.prt");						// 待压缩的PRT档案
	string sPartZip = cCurrentFolder + "\\data\\CS2020-C006-CNC-T_PRT.zip";			
	uploadFile(theSession,sGroupId,myTask,vParts,sPartZip,cncTemplateId,"PRT");	

	//	创建 3D 预览图片
	vector<string> v3DJpgs;
	v3DJpgs.push_back(cCurrentFolder+"\\data\\temp\\CS2020-C006-CNC-T.jpg");				// 待压缩的3D预览图片
	string s3DJpgZip = cCurrentFolder + "\\data\\CS2020-C006-CNC-T_PARTVIEW.zip";			
	uploadFile(theSession,sGroupId,myTask,v3DJpgs,s3DJpgZip,cncTemplateId,"PARTVIEW");

	//	创建 装夹图
	vector<string> v2DJpgs;
	v2DJpgs.push_back(cCurrentFolder+"\\data\\temp\\CS2020-C006-CNC-T_NC_SETUP.cgm");		// 待压缩的装夹图cgm
	v2DJpgs.push_back(cCurrentFolder+"\\data\\temp\\CS2020-C006-CNC-T_NC_SETUP.jpg");		// 待压缩的装夹图jpg
	string s2DJpgZip = cCurrentFolder + "\\data\\CS2020-C006-CNC-T_SETUP.zip";			
	uploadFile(theSession,sGroupId,myTask,v2DJpgs,s2DJpgZip,cncTemplateId,"SETUP");

	//	创建 STL
	vector<string> vSTLs;
	vSTLs.push_back(cCurrentFolder+"\\data\\temp\\CS2020-C006-CNC-T.stl");					// 待压缩的工件的stl档案
	vSTLs.push_back(cCurrentFolder+"\\data\\temp\\CS2020-C006-CNC-T_Blank.stl");			// 待压缩的工件毛坯的stl档案
	string sSTLZip = cCurrentFolder + "\\data\\CS2020-C006-CNC-T_STL.zip";			
	uploadFile(theSession,sGroupId,myTask,vSTLs,sSTLZip,cncTemplateId,"STL");

	//	创建 刀路预览图片
	vector<string> vSHOPDOCs;
	vSHOPDOCs.push_back(cCurrentFolder+"\\data\\toolpath\\杀顶面_F1_path.jpg");				// 待压缩的刀路预览图片
	vSHOPDOCs.push_back(cCurrentFolder+"\\data\\toolpath\\电极头开粗_F2_path.jpg");			// 待压缩的刀路预览图片
	vSHOPDOCs.push_back(cCurrentFolder+"\\data\\toolpath\\基座开粗_F3_path.jpg");				// 待压缩的刀路预览图片
	vSHOPDOCs.push_back(cCurrentFolder+"\\data\\toolpath\\光平面_F4_path.jpg");				// 待压缩的刀路预览图片
	vSHOPDOCs.push_back(cCurrentFolder+"\\data\\toolpath\\光直面_F5_path.jpg");				// 待压缩的刀路预览图片
	vSHOPDOCs.push_back(cCurrentFolder+"\\data\\toolpath\\光基准面_F6_path.jpg");				// 待压缩的刀路预览图片
	vSHOPDOCs.push_back(cCurrentFolder+"\\data\\toolpath\\光基座侧壁_F7_path.jpg");			// 待压缩的刀路预览图片
	string sSHOPDOCZip = cCurrentFolder + "\\data\\CS2020-C006-CNC-T_SHOPDOC.zip";			
	uploadFile(theSession,sGroupId,myTask,vSHOPDOCs,sSHOPDOCZip,cncTemplateId,"SHOPDOC","F");

	//	创建 NC文件
	vector<string> vNCs;
	vNCs.push_back(cCurrentFolder+"\\data\\FANUC_CU\\CS2020-C006-1.NC");					//	待压缩的NC文件
	vNCs.push_back(cCurrentFolder+"\\data\\FANUC_CU\\CS2020-C006-2.NC");					//	待压缩的NC文件
	string sNCZip = cCurrentFolder + "\\data\\CS2020-C006-CNC-T_NC.zip";
	string fileNames="";
	for each (string s in vNCs)
	{
		fileNames.append(s.substr(s.rfind('\\')+1)+",");
	}
	uploadFile(theSession,sGroupId,myTask,vNCs,sNCZip,cncTemplateId,"NC","F",fileNames.substr(0,fileNames.rfind(',')));

	ECHO(">>> NC文件上传成功！");
}

// 任务完工
bool finishTask(MdbSession& theSession,pWebCAMTask myTask)
{
	bool isOK = false;
	if (doFinishCAMTask)
	{
		if(theSession.finishTask(myTask))
		{
			isOK = true;
			ECHO(">>> CAM任务完工 OK！");
		}
		else
			ECHO("### CAM任务完工失败！");	
	}	
	else
	{
		isOK = true;
	}
	return isOK;
}

//	获取 程式 类型
pCncTemplate getCNCTemplate(MdbSession& theSession,pWebCAMTask myTask)
{
	vector<pCncTemplate> vTemplates = theSession.getNCTempaltes(myTask);
	if (vTemplates.size()<1)
		EXCEPT("### 获取程式类型失败！");

	//	获取 程式类型
	pCncTemplate currentTemplate;
	currentTemplate.reset();
	for each (pCncTemplate t in vTemplates)
	{
		if (t->postName == cncPostName)
		{
			currentTemplate = t;
			break;
		}
	}
	if (currentTemplate == NULL)
		currentTemplate = vTemplates[0];

	return currentTemplate;
}

int _tmain(int argc, _TCHAR* argv[])
{
	bool isOK = false;
	try
	{
		MdbSession::initLogFun(showLog,showLog,showLog,showExcept,showLog); // 初始化 日志

		MdbSession& theSession = MdbSession::getSession(userName.c_str(),password.c_str()); 
		
		cCurrentFolder = GetCurrentDir();

		pWebCAMTask myTask;
		myTask.reset();

		//	获取 当前登录 用户的 CAM 任务
		vector<pWebCAMTask> tasks=theSession.getCAMTasks();
		for each (pWebCAMTask t in tasks)
		{
			if (myTask==NULL && t->moldNo == moldNo && t->partNo == partNo)
				myTask = t;
			//ECHO(">>> %s %s %s %s %s",t->moldNo.c_str(),t->partNo.c_str(),t->processObject.c_str(),t->processName.c_str(),t->phaseName.c_str());
		}

		string fileId = myTask->fileIdBomPart;

		if (myTask==NULL)
			EXCEPT("### 没有找到任务！");
		else if (myTask->taskStatus == 100)
			EXCEPT("### 任务已完工！请申请异常后再上传！");
		
		ECHO(">>> 任务信息：moldNo=%s partNo=%s processName=%s phaseName=%s status=%d fileId=%s",myTask->moldNo.c_str(),myTask->partNo.c_str(),myTask->processName.c_str(),myTask->phaseName.c_str(),myTask->taskStatus,fileId.c_str());

		//	下载 任务档案
		string targetFolder = targetLocalFolder;
		string sfile = theSession.downloadCamTaskFile(myTask,targetFolder);
		ECHO(">>> 档案下载完毕 ： %s",sfile.c_str());
		
		//	获取 程式类型
		pCncTemplate currentTemplate = getCNCTemplate(theSession,myTask);

		//	NC上传
		ECHO(">>> 正在上传NC数据...");
		string sGroupId = "";
		if (uploadNCData(theSession,sGroupId))
		{
			ECHO(">>> NC数据上传成功 : %s",sGroupId.c_str());

			uploadNCFile(theSession,sGroupId,myTask,currentTemplate->typeId);

			isOK = finishTask(theSession,myTask);
		}		
		else
		{
			ECHO("### NC数据上传失败！");
		}
	}
	catch (exception &e)
	{
		ECHO("### 异常了 ： %s",e.what());
	}
	catch(...)
	{
		ECHO("### 异常了 ： 未知异常！");
	}
	
	if (isOK)
	{
		ECHO(">>> 执行完毕！");
		int i=0;
		while (i++ < 3)
		{
			Sleep(1000);
		}
	}
	else
	{
		system("pause");
	}

	return 0;
}

