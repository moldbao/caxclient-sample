// MdbOpenApiTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <MdbSession.h>
#include "Json/json.h"

#include "MdbGlogHelper.hxx"
void showLog(char *s){	GINFO << s;}
void showExcept(char *s){	GERROR << s;}

const string userName = "";					//	�û���	| Ϊ��ʱ �������ļ��м���
const string password = "";					//	����		| Ϊ��ʱ �������ļ��м���
const string moldNo = "CS2020";				//	Ŀ�� ��Ʒ��
const string partNo = "C006";				//	Ŀ�� ������
const string targetLocalFolder = "D:\\";	//	���ر���Ŀ¼
const string cncPostName = "FANUC_NC";		//	��������
const bool doFinishCAMTask = false;			//	�Ƿ���ҪCAM�����Զ��깤

string cCurrentFolder = "";

//	��ȡ NC ��ʽ����ϸ������Ϣ | �˴�Ϊ��ʾ���룬��json�ļ��м��أ�ʵ�����CAX����л�ȡ
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

//	�ϴ� NC ����
bool uploadNCData(MdbSession& theSession,string &groupId)
{
	try
	{
		Json::Value jNCInfo = getNCInfo(theSession.isTest);
		if (jNCInfo.empty())
			EXCEPT("### ��ȡNC��Ϣʧ�ܣ�");

		string sResult="";
		map<string,string> mNCIDs = theSession.uploadNCInfo(Json::FastWriter().write(jNCInfo));
		if (mNCIDs.size()!=1)
			EXCEPT("### NC�����ϴ�ʧ�� �� mNCIDs.size() = %d",mNCIDs.size());

		groupId = mNCIDs.begin()->second;
		return true;
	}
	catch (exception &e)
	{
		EXCEPT("### �ϴ�NC����ʧ�� �� %s",e.what());
	}
	return false;
}

//	���� ѹ���� �� �ϴ�
void uploadFile(MdbSession& theSession,string &sGroupId,pWebCAMTask myTask,vector<string> &vFiles,string sZip,string sProgramTypeId,string fileType,string sparkType="",string fileNames="")
{
	string sPartShortName = sZip.substr(sZip.rfind('\\')+1);

	ECHO(">>> ����ѹ�� %s ...",sPartShortName.c_str());
	if(!ZipFiles(vFiles,sZip))
		EXCEPT("### ����ѹ���� %s ʧ�ܣ�",sPartShortName.c_str());

	ECHO(">>> �����ϴ� %s ...",sPartShortName.c_str());
	if(!theSession.uploadNCFile(myTask,sGroupId,sProgramTypeId,fileType,sZip,sparkType,fileNames))
		EXCEPT("### �ϴ��ļ� %s ʧ�ܣ�",sPartShortName.c_str());
}

//	�ϴ� NC �ļ�
void uploadNCFile(MdbSession& theSession,string &sGroupId,pWebCAMTask myTask,string cncTemplateId)
{
	//	�������� PRT ѹ����
	vector<string> vParts;
	vParts.push_back(cCurrentFolder+"\\data\\CS2020-C006-CNC-T.prt");						// ��ѹ����PRT����
	string sPartZip = cCurrentFolder + "\\data\\CS2020-C006-CNC-T_PRT.zip";			
	uploadFile(theSession,sGroupId,myTask,vParts,sPartZip,cncTemplateId,"PRT");	

	//	���� 3D Ԥ��ͼƬ
	vector<string> v3DJpgs;
	v3DJpgs.push_back(cCurrentFolder+"\\data\\temp\\CS2020-C006-CNC-T.jpg");				// ��ѹ����3DԤ��ͼƬ
	string s3DJpgZip = cCurrentFolder + "\\data\\CS2020-C006-CNC-T_PARTVIEW.zip";			
	uploadFile(theSession,sGroupId,myTask,v3DJpgs,s3DJpgZip,cncTemplateId,"PARTVIEW");

	//	���� װ��ͼ
	vector<string> v2DJpgs;
	v2DJpgs.push_back(cCurrentFolder+"\\data\\temp\\CS2020-C006-CNC-T_NC_SETUP.cgm");		// ��ѹ����װ��ͼcgm
	v2DJpgs.push_back(cCurrentFolder+"\\data\\temp\\CS2020-C006-CNC-T_NC_SETUP.jpg");		// ��ѹ����װ��ͼjpg
	string s2DJpgZip = cCurrentFolder + "\\data\\CS2020-C006-CNC-T_SETUP.zip";			
	uploadFile(theSession,sGroupId,myTask,v2DJpgs,s2DJpgZip,cncTemplateId,"SETUP");

	//	���� STL
	vector<string> vSTLs;
	vSTLs.push_back(cCurrentFolder+"\\data\\temp\\CS2020-C006-CNC-T.stl");					// ��ѹ���Ĺ�����stl����
	vSTLs.push_back(cCurrentFolder+"\\data\\temp\\CS2020-C006-CNC-T_Blank.stl");			// ��ѹ���Ĺ���ë����stl����
	string sSTLZip = cCurrentFolder + "\\data\\CS2020-C006-CNC-T_STL.zip";			
	uploadFile(theSession,sGroupId,myTask,vSTLs,sSTLZip,cncTemplateId,"STL");

	//	���� ��·Ԥ��ͼƬ
	vector<string> vSHOPDOCs;
	vSHOPDOCs.push_back(cCurrentFolder+"\\data\\toolpath\\ɱ����_F1_path.jpg");				// ��ѹ���ĵ�·Ԥ��ͼƬ
	vSHOPDOCs.push_back(cCurrentFolder+"\\data\\toolpath\\�缫ͷ����_F2_path.jpg");			// ��ѹ���ĵ�·Ԥ��ͼƬ
	vSHOPDOCs.push_back(cCurrentFolder+"\\data\\toolpath\\��������_F3_path.jpg");				// ��ѹ���ĵ�·Ԥ��ͼƬ
	vSHOPDOCs.push_back(cCurrentFolder+"\\data\\toolpath\\��ƽ��_F4_path.jpg");				// ��ѹ���ĵ�·Ԥ��ͼƬ
	vSHOPDOCs.push_back(cCurrentFolder+"\\data\\toolpath\\��ֱ��_F5_path.jpg");				// ��ѹ���ĵ�·Ԥ��ͼƬ
	vSHOPDOCs.push_back(cCurrentFolder+"\\data\\toolpath\\���׼��_F6_path.jpg");				// ��ѹ���ĵ�·Ԥ��ͼƬ
	vSHOPDOCs.push_back(cCurrentFolder+"\\data\\toolpath\\��������_F7_path.jpg");			// ��ѹ���ĵ�·Ԥ��ͼƬ
	string sSHOPDOCZip = cCurrentFolder + "\\data\\CS2020-C006-CNC-T_SHOPDOC.zip";			
	uploadFile(theSession,sGroupId,myTask,vSHOPDOCs,sSHOPDOCZip,cncTemplateId,"SHOPDOC","F");

	//	���� NC�ļ�
	vector<string> vNCs;
	vNCs.push_back(cCurrentFolder+"\\data\\FANUC_CU\\CS2020-C006-1.NC");					//	��ѹ����NC�ļ�
	vNCs.push_back(cCurrentFolder+"\\data\\FANUC_CU\\CS2020-C006-2.NC");					//	��ѹ����NC�ļ�
	string sNCZip = cCurrentFolder + "\\data\\CS2020-C006-CNC-T_NC.zip";
	string fileNames="";
	for each (string s in vNCs)
	{
		fileNames.append(s.substr(s.rfind('\\')+1)+",");
	}
	uploadFile(theSession,sGroupId,myTask,vNCs,sNCZip,cncTemplateId,"NC","F",fileNames.substr(0,fileNames.rfind(',')));

	ECHO(">>> NC�ļ��ϴ��ɹ���");
}

// �����깤
bool finishTask(MdbSession& theSession,pWebCAMTask myTask)
{
	bool isOK = false;
	if (doFinishCAMTask)
	{
		if(theSession.finishTask(myTask))
		{
			isOK = true;
			ECHO(">>> CAM�����깤 OK��");
		}
		else
			ECHO("### CAM�����깤ʧ�ܣ�");	
	}	
	else
	{
		isOK = true;
	}
	return isOK;
}

//	��ȡ ��ʽ ����
pCncTemplate getCNCTemplate(MdbSession& theSession,pWebCAMTask myTask)
{
	vector<pCncTemplate> vTemplates = theSession.getNCTempaltes(myTask);
	if (vTemplates.size()<1)
		EXCEPT("### ��ȡ��ʽ����ʧ�ܣ�");

	//	��ȡ ��ʽ����
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
		MdbSession::initLogFun(showLog,showLog,showLog,showExcept,showLog); // ��ʼ�� ��־

		MdbSession& theSession = MdbSession::getSession(userName.c_str(),password.c_str()); 
		
		cCurrentFolder = GetCurrentDir();

		pWebCAMTask myTask;
		myTask.reset();

		//	��ȡ ��ǰ��¼ �û��� CAM ����
		vector<pWebCAMTask> tasks=theSession.getCAMTasks();
		for each (pWebCAMTask t in tasks)
		{
			if (myTask==NULL && t->moldNo == moldNo && t->partNo == partNo)
				myTask = t;
			//ECHO(">>> %s %s %s %s %s",t->moldNo.c_str(),t->partNo.c_str(),t->processObject.c_str(),t->processName.c_str(),t->phaseName.c_str());
		}

		string fileId = myTask->fileIdBomPart;

		if (myTask==NULL)
			EXCEPT("### û���ҵ�����");
		else if (myTask->taskStatus == 100)
			EXCEPT("### �������깤���������쳣�����ϴ���");
		
		ECHO(">>> ������Ϣ��moldNo=%s partNo=%s processName=%s phaseName=%s status=%d fileId=%s",myTask->moldNo.c_str(),myTask->partNo.c_str(),myTask->processName.c_str(),myTask->phaseName.c_str(),myTask->taskStatus,fileId.c_str());

		//	���� ���񵵰�
		string targetFolder = targetLocalFolder;
		string sfile = theSession.downloadCamTaskFile(myTask,targetFolder);
		ECHO(">>> ����������� �� %s",sfile.c_str());
		
		//	��ȡ ��ʽ����
		pCncTemplate currentTemplate = getCNCTemplate(theSession,myTask);

		//	NC�ϴ�
		ECHO(">>> �����ϴ�NC����...");
		string sGroupId = "";
		if (uploadNCData(theSession,sGroupId))
		{
			ECHO(">>> NC�����ϴ��ɹ� : %s",sGroupId.c_str());

			uploadNCFile(theSession,sGroupId,myTask,currentTemplate->typeId);

			isOK = finishTask(theSession,myTask);
		}		
		else
		{
			ECHO("### NC�����ϴ�ʧ�ܣ�");
		}
	}
	catch (exception &e)
	{
		ECHO("### �쳣�� �� %s",e.what());
	}
	catch(...)
	{
		ECHO("### �쳣�� �� δ֪�쳣��");
	}
	
	if (isOK)
	{
		ECHO(">>> ִ����ϣ�");
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

