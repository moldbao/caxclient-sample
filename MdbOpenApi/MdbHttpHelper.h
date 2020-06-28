#pragma once

#include "stdafx.h"

#include "MdbConstants/MdbConstants.h"

#include "MdbWebCamTask.h"

class MDBAPI MdbHttpHelper
{
public:
	~MdbHttpHelper(void);
	
	MdbHttpHelper(const char *_userName="",const char *_password="");

	static MdbConstants *mdbConstants;
	
	//	���� http post ����
	int httpPost(const string &Page,const string &Para,string &Result,bool bGetCookies=false,string sContentType="application/json");

	//	ָ�� ip �� �˿� ���� http post ����
	int httpPost(const string &ip,const string &port,const string &Page,const string &Para,string &Result,bool bGetCookies=false,string sContentType="application/json");

	//	�ļ� ����
	bool downloadFile(const string &fileId,const string &SaveFolder,string &outFile);

	//	�ļ� �ϴ�
	bool uploadFile(const string &sFile,const string &Paras);

	//	�ϴ� NC ��Ϣ
	map<string,string>  uploadNCInfo(string &para);	

	//	��ȡ CAM ����
	vector<pWebCAMTask> MdbHttpHelper::getCAMTask(const string &sSession="CNC",const string &smoldNo="",const string &spartNo="",const string &sprocessObject="");

	//	��ȡ JSESSIONID �� ֵ
	string getJsessionId();

	//	��� �Ƿ� Ϊ ����ϵͳ �� IP ���� www.moldyun.com
	bool isTest();

	//	���� �깤
	bool finishTask( pWebCAMTask myTask );
	
	//	��ȡ ��ʽ����
	vector<pCncTemplate> getNCTempaltes(pWebCAMTask myTask,string sectionType="CNC");

	//	��ȡ ����� �����б�
	map<string,string> getPartFiles(string partId);
private:
	string ip;				//	ϵͳ��IP
	string port;			//	ϵͳ�Ķ˿�

	string cookies;			//	���� cookies
	string jsessionId;		//	cookies�е� JSESSIONID

	string userName;		//	�û���
	string userRealName;	//	����
	string password;		//	����
	string tenantId;		//	�⻧Id

	string localIp;			//	�������ص�IP
	string localPort;		//	�������صĶ˿�
	string localConnect;	//	�������ز������� url

	string loginUrl;		//	�û���¼
	string uploadUrl;		//	�ļ��ϴ�	
	string downloadUrl;		//	�ļ�����
	string camTaskUrl;		//	��ȡCAM����
	string cncUploadUrl;	//	�ϴ�NC����
	string camFinishUrl;	//	CAM�����깤
	string cncTemplateUrl;	//	��ȡ��ʽ����
	string getPartFileUrl;	//	��ȡ��������б�

	bool useLocalGateway;	//	�Ƿ� ʹ�� ��������
private:

	//	�û���¼ �������ʱ���Զ���¼�������¼�� cookies
	bool userLogin();
	
	//	���� ���� ���� �Ƿ� ��ͨ
	bool checkLocalGateway();

	// �ж� ��ʽ���� �Ƿ� �Ѿ� ������ �б��� �� ��ʽ�ϴ�ʱ ��ͬ��ʽ���� �� ����һ�μ���
	bool isExistNCTemplate(vector<pCncTemplate> &vCncTempaltes,pCncTemplate t);
};

