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
	
	//	发送 http post 请求
	int httpPost(const string &Page,const string &Para,string &Result,bool bGetCookies=false,string sContentType="application/json");

	//	指定 ip 和 端口 发送 http post 请求
	int httpPost(const string &ip,const string &port,const string &Page,const string &Para,string &Result,bool bGetCookies=false,string sContentType="application/json");

	//	文件 下载
	bool downloadFile(const string &fileId,const string &SaveFolder,string &outFile);

	//	文件 上传
	bool uploadFile(const string &sFile,const string &Paras);

	//	上传 NC 信息
	map<string,string>  uploadNCInfo(string &para);	

	//	获取 CAM 任务
	vector<pWebCAMTask> MdbHttpHelper::getCAMTask(const string &sSession="CNC",const string &smoldNo="",const string &spartNo="",const string &sprocessObject="");

	//	获取 JSESSIONID 的 值
	string getJsessionId();

	//	检测 是否 为 测试系统 ： IP 不是 www.moldyun.com
	bool isTest();

	//	任务 完工
	bool finishTask( pWebCAMTask myTask );
	
	//	获取 程式类型
	vector<pCncTemplate> getNCTempaltes(pWebCAMTask myTask,string sectionType="CNC");

	//	获取 零件的 附件列表
	map<string,string> getPartFiles(string partId);
private:
	string ip;				//	系统的IP
	string port;			//	系统的端口

	string cookies;			//	完整 cookies
	string jsessionId;		//	cookies中的 JSESSIONID

	string userName;		//	用户名
	string userRealName;	//	姓名
	string password;		//	密码
	string tenantId;		//	租户Id

	string localIp;			//	本地网关的IP
	string localPort;		//	本地网关的端口
	string localConnect;	//	本地网关测试连接 url

	string loginUrl;		//	用户登录
	string uploadUrl;		//	文件上传	
	string downloadUrl;		//	文件下载
	string camTaskUrl;		//	获取CAM任务
	string cncUploadUrl;	//	上传NC数据
	string camFinishUrl;	//	CAM任务完工
	string cncTemplateUrl;	//	获取程式类型
	string getPartFileUrl;	//	获取零件附件列表

	bool useLocalGateway;	//	是否 使用 本地网关
private:

	//	用户登录 ：构造的时候自动登录，保存登录的 cookies
	bool userLogin();
	
	//	测试 本地 网关 是否 联通
	bool checkLocalGateway();

	// 判断 程式类型 是否 已经 包含在 列表中 ： 程式上传时 相同程式类型 仅 出现一次即可
	bool isExistNCTemplate(vector<pCncTemplate> &vCncTempaltes,pCncTemplate t);
};

