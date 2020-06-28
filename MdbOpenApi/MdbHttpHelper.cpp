#include "MdbHttpHelper.h"
#include <afxinet.h>
#include "json/json.h"
#include "MdbCommon.h"

MdbConstants* MdbHttpHelper::mdbConstants = NULL;

#include <afxinet.h> 
#define RECVPACK_SIZE (1024*500)			//	每次读取 500 KB

MdbHttpHelper::MdbHttpHelper(const char *_userName/*=""*/,const char *_password/*=""*/ )
{
	//printf("+++ MdbHttpHelper\n");

	if (mdbConstants==NULL)
		EXCEPT("### mdbContants 未初始化！");

	if (strlen(_userName)>0 && strlen(_password)>0)
	{
		userName = _userName;
		password = _password;
	}
	else
	{
		userName = mdbConstants->constantsMap["TEST_USERNAME"].GetBuffer();
		password = mdbConstants->constantsMap["TEST_PASSWORD"].GetBuffer();
	}

	ip = mdbConstants->constantsMap["ELE_SERVER_IP"].GetBuffer();

	port = mdbConstants->constantsMap["ELE_SERVER_PORT"].GetBuffer();

	loginUrl = mdbConstants->constantsMap["ELE_USER_LOGIN"].GetBuffer();

	uploadUrl = mdbConstants->constantsMap["BmsFileUpload"].GetBuffer();

	downloadUrl = mdbConstants->constantsMap["DOWNLOADFILE"].GetBuffer();

	camTaskUrl = mdbConstants->constantsMap["ELE_CAM_TASK_PAGE"].GetBuffer();

	cncUploadUrl = mdbConstants->constantsMap["CNCUPLOAD"].GetBuffer();

	camFinishUrl = mdbConstants->constantsMap["CAMFINISH"].GetBuffer();

	cncTemplateUrl = mdbConstants->constantsMap["CNCTemplate"].GetBuffer();
	
	getPartFileUrl = mdbConstants->constantsMap["GetPartFile"].GetBuffer();

	useLocalGateway = false;

	localConnect = "gateway/connect";
		
	userLogin();

	jsessionId = cookies.substr(cookies.find("JSESSIONID="));  // JSESSIONID=9218515f-1f5c-45db-9b04-d6f2df7880e1
	jsessionId = jsessionId.substr(0,jsessionId.find(";"));

	if (useLocalGateway)
	{
		uploadUrl = "gateway/file/upload/common";
		downloadUrl = "gateway/file/getfile.do?";
	}
	else
	{
		localIp = ip;
		localPort = port;
	}
}

MdbHttpHelper::~MdbHttpHelper(void)
{
	//printf("--- MdbHttpHelper\n");
}


bool MdbHttpHelper::checkLocalGateway()
{
	Json::Value jObj;
	jObj["tenantId"]=tenantId;
	string result="";
	if(httpPost(localIp,localPort,localConnect,Json::FastWriter().write(jObj),result)==0)
	{
		Json::Reader reader;
		Json::Value jv;			
		if (reader.parse(result,jv))
		{   
			if (jv["code"].asInt()==1)
				return true;
		}
	}
	ECHO("### 本地网关测试不通！");
	return false;
}

bool MdbHttpHelper::userLogin()
{
	Json::Value jObj;
	jObj["phoneNum"]=userName;
	jObj["password"]=password;
	Json::FastWriter fw;
	string sJson = fw.write(jObj);
	string sResult;
	int resp = 0;

	try
	{
		resp = httpPost(loginUrl,sJson,sResult,true);
	}
	catch (exception &e)
	{
		EXCEPT("### 用户登录 发送http请求异常 ： %s",e.what());
	}
	catch(...)
	{
		EXCEPT("### 用户登录 发送http请求异常 ： 未知异常！");
	}		

	if (resp == 0)
	{
		Json::Reader reader;
		Json::Value jv;	
		if(reader.parse(sResult,jv))
		{
			if(jv["code"].asInt() == 1)
			{
				Json::Value userData = jv["data"]["userData"];
				userRealName = userData["realName"].asString();
				tenantId = userData["tenantId"].asString();

				if(userData.isMember("currentGateway") && userData["currentGateway"].isObject())
				{
					Json::Value currentGateway = userData["currentGateway"];
					localIp = currentGateway["gatewayIp"].asString();
					localPort = currentGateway["gatewayPort"].asString();
					if (checkLocalGateway())
						useLocalGateway = true;
				}
				return true;
			}
			else
			{
				EXCEPT("### 用户登录 失败 : %s",jv["message"].asString().c_str());
			}
		}
		else
		{
			EXCEPT("### 用户登录 返回参数解析失败 ： %s",reader.getFormattedErrorMessages().c_str());
		}
	}		
	else
	{
		EXCEPT("### 用户登录失败！");
	}
}

int MdbHttpHelper::httpPost(const string &Page,const string &Para,string &Result,bool bGetCookies/*=false*/,string sContentType/*="application/json"*/)
{
	return httpPost(ip,port,Page,Para,Result,bGetCookies,sContentType);
}

int MdbHttpHelper::httpPost( const string &ip,const string &port,const string &Page,const string &Para,string &Result,bool bGetCookies/*=false*/,string sContentType/*="application/json"*/ )
{
	try
	{
		CInternetSession m_InetSession(_T("session"),0,INTERNET_OPEN_TYPE_PRECONFIG,NULL,NULL,INTERNET_FLAG_DONT_CACHE);     //设置不缓冲

		CHttpConnection* pServer = NULL;
		CHttpFile* pFile = NULL;
		CString strHtml = "";
		CString strRequest = Para.size()>0 ? AnsitoUtf8(Para.c_str()).c_str() : "{}"; //POST过去的数据

		LOG(">>> url  : http://%s:%s/%s",ip.c_str(),port.c_str(),Page.c_str());
		LOG(">>> para : %s",Para.c_str());

		int iWaitTime = 20 * 60 * 1000 ;// 20分钟 
		m_InetSession.SetOption(INTERNET_OPTION_CONNECT_TIMEOUT, iWaitTime);		// 5000毫秒的连接超时
		m_InetSession.SetOption(INTERNET_OPTION_SEND_TIMEOUT, iWaitTime);			// 5000毫秒的发送超时
		m_InetSession.SetOption(INTERNET_OPTION_RECEIVE_TIMEOUT, iWaitTime);		// 5000毫秒的接收超时
		m_InetSession.SetOption(INTERNET_OPTION_DATA_SEND_TIMEOUT, iWaitTime);		// 5000毫秒的发送超时
		m_InetSession.SetOption(INTERNET_OPTION_DATA_RECEIVE_TIMEOUT, iWaitTime);	// 5000毫秒的接收超时
		m_InetSession.SetOption(INTERNET_OPTION_CONNECT_RETRIES, 1);				// 1次重试

		INTERNET_PORT nPort = atoi(port.c_str());
		pServer = m_InetSession.GetHttpConnection(ip.c_str(), nPort);
		if (pServer==NULL)
			throw exception("GetHttpConnection 服务器连接失败！");

		pFile = pServer->OpenRequest(CHttpConnection::HTTP_VERB_POST, (Page[0]=='/'?(Page):("/"+Page)).c_str(),NULL,1,NULL,NULL,INTERNET_FLAG_NO_COOKIES); // 打开连接
		if (pFile == NULL)
			throw exception("OpenRequest 打开Url失败！");

		CString strHeaders = ("Accept: image/png,*/*;q=0.5\r\nAccept-Language: zh-cn,zh;q=0.5\r\nAccept-Encoding: gzip,deflate\r\nAccept-Charset: gb2312,utf-8;q=0.7,*;q=0.7\r\nKeep-Alive: 30000\r\nConnection: keep-alive\r\nContent-Type: "+sContentType+";charset=UTF-8").c_str();
		pFile->AddRequestHeaders(strHeaders);
		string sOldCookies = cookies;
		if (cookies.size()>0)
			pFile->AddRequestHeaders(CString("Cookie: ") + cookies.c_str());

		string sOs = getOsInfo();
		if (sOs.size()<1)
			sOs = "Windows NT 6.1";

		sOs = "Cax/5.0 (" + sOs + "; WOW64) Application CAX/NX";
		pFile->AddRequestHeaders(CString("User-Agent: ") + A2T((char *)sOs.c_str()));

		int iLen = strRequest.GetLength();
		pFile->SendRequestEx(iLen);
		pFile->WriteString(strRequest); 
		pFile->EndRequest();

		DWORD dwRet;
		pFile->QueryInfoStatusCode(dwRet);

		int iResp = -1;
		if (dwRet == HTTP_STATUS_OK)
		{
			CString strLine;
			while (pFile->ReadString(strLine))	
			{
				strHtml += strLine;
			}
			Result = Utf8toAnsi(string(CStringA(strHtml)));
			LOG(">>> resp : %s",Result.c_str());
			if (bGetCookies)// 取返回的COOKIE
			{				
				CString strInfo ;
				DWORD dw = 0 ;
				pFile->QueryInfo(HTTP_QUERY_SET_COOKIE ,strInfo ,&dw) ;
				cookies = string(CStringA(strInfo));
				if (cookies.size()<1)
					cookies = sOldCookies;
			}
			iResp = 0;
		} 
		else
		{
			EXCEPT("### httpPost 访问 %s:%s/%s 异常 ： %d",ip.c_str(),port.c_str(),Page.c_str(),dwRet);
		}
		delete pFile;
		delete pServer;
		return iResp;
	}
	catch (CException* e){
		TCHAR szError[MAX_PATH];
		e->GetErrorMessage(szError, MAX_PATH);
		e->Delete();
		EXCEPT("httpPost 通信异常！\n%s",szError);
	}
	catch(exception &e)
	{
		EXCEPT("### httpPost Error : %s",e.what());
	}
	catch(...)
	{
		EXCEPT("### httpPost unKnown Error!");
	}
}

string getHttpOriginName(CHttpFile* pFile)
{
	/*
	HTTP/1.1 200
	Access-Control-Allow-Credentials: true
	Access-Control-Allow-Methods: POST, GET, OPTIONS, DELETE
	Access-Control-Max-Age: 3600
	Access-Control-Allow-Headers: x-requested-with
	Pragma: No-cache
	Cache-Control: no-cache
	Expires: Thu, 01 Jan 1970 00:00:00 GMT
	Content-Disposition: attachment;filename=CN181107-A01-EDM-T.prt
	Content-Type: application/unigraphics
	Transfer-Encoding: chunked
	Date: Wed, 07 Nov 2018 08:14:20 GMT
	*/
	CString header="";
	string sFileName = "";
	pFile->QueryInfo( HTTP_QUERY_RAW_HEADERS_CRLF,header);
	int ifindex = header.Find("filename=");
	if (ifindex>0)
	{
		ifindex = header.Find("=",ifindex)+1;
		int ifend = header.Find("\n",ifindex);
		CString smf = header.Mid(ifindex,ifend-ifindex-1);
		LOG(">>> http header filename: BEGIN|%s|END",smf.GetBuffer());
		sFileName = smf.GetBuffer();
	}
	return sFileName;
}

bool MdbHttpHelper::downloadFile(const string &fileId,const string &SaveFolder,string &outFile)
{
	try
	{
		string url = downloadUrl + "fileId="+fileId + "&"+jsessionId;
		LOG(">>> http://%s:%s/%s",localIp.c_str(),localPort.c_str(),url.c_str());
		CInternetSession Session("session", 0, INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, INTERNET_FLAG_DONT_CACHE);     //设置不缓冲

		int iWaitTime = 20 * 60 * 1000 ;// 20分钟 
		Session.SetOption(INTERNET_OPTION_CONNECT_TIMEOUT, iWaitTime);		// 5000毫秒的连接超时
		Session.SetOption(INTERNET_OPTION_SEND_TIMEOUT, iWaitTime);			// 5000毫秒的发送超时
		Session.SetOption(INTERNET_OPTION_RECEIVE_TIMEOUT, iWaitTime);		// 5000毫秒的接收超时
		Session.SetOption(INTERNET_OPTION_DATA_SEND_TIMEOUT, iWaitTime);	// 5000毫秒的发送超时
		Session.SetOption(INTERNET_OPTION_DATA_RECEIVE_TIMEOUT, iWaitTime);	// 5000毫秒的接收超时
		Session.SetOption(INTERNET_OPTION_CONNECT_RETRIES, 1);				// 1次重试

		INTERNET_PORT nPort = atoi(localPort.c_str());
		CHttpConnection* pServer = Session.GetHttpConnection(localIp.c_str(), nPort);
		if (pServer == NULL)
			throw exception("GetHttpConnection 服务器连接失败！");		 
		CHttpFile* pFile = pServer->OpenRequest(CHttpConnection::HTTP_VERB_GET, url.c_str(), NULL,NULL,NULL,NULL,INTERNET_FLAG_EXISTING_CONNECT|INTERNET_FLAG_NO_COOKIES); // 打开连接
		if (pFile == NULL)
			throw exception("OpenRequest 打开Url失败！");

		pFile->AddRequestHeaders(A2T("Accept: image/png,*/*;q=0.5"));
		if (cookies.size()>0)
			pFile->AddRequestHeaders(CString("Cookie: ") + cookies.c_str());	
		pFile->SendRequest();
		DWORD dwRet;
		pFile->QueryInfoStatusCode(dwRet);
		int iResp = -1;
		if (dwRet == HTTP_STATUS_OK)
		{
			outFile = SaveFolder + "\\" + getHttpOriginName(pFile);
			remove(outFile.c_str());			 

			CFile file(outFile.c_str(), CFile::modeCreate | CFile::modeWrite);
			char *sr = new char[RECVPACK_SIZE+1];
			while (1) {
				int n = pFile->Read(sr,RECVPACK_SIZE);				 
				if (n <= 0) break;//
				file.Write(sr,n);
			} 
			__int64 iFileSize = file.GetLength();
			file.Close();
			delete[] sr;	

			if (iFileSize < 1010) // 小于 1 kb
			{
				EXCEPT("### Http 下载文件出错，%s 文件大小 = %I64u ，可能不是合法的数据文件！",outFile.c_str(),iFileSize);
				return false;
			}
			else
			{
				LOG(">>> 下载成功 ： %s",outFile.c_str());
				return true;
			}
		}
		else
		{
			EXCEPT("### [%d]%s Http 异常 http://%s:%s/%s",dwRet,dwRet==404?"(文件不存在)":"",localIp.c_str(),localPort.c_str(),url.c_str());
			return false;
		}		 
		pFile->Close();
		delete pFile ;  
		delete pServer;
	}
	catch (CException* e){
		TCHAR szError[MAX_PATH];
		e->GetErrorMessage(szError, MAX_PATH);
		e->Delete();
		EXCEPT("### downloadFile Error : %s",szError);
	}
	catch(exception &ee)
	{
		EXCEPT("### downloadFile Error : %s",ee.what());
	}
	catch(...)
	{
		EXCEPT("### downloadFile unKnown Error!");
	}
	return false;
}

bool MdbHttpHelper::uploadFile(const string &sFile,const string &Paras)
{
	bool res = false;
	//HTTP服务
	CHttpConnection* m_pHC=NULL;
	CHttpFile* m_pHF=NULL;
	CInternetSession m_cis(_T("session"), 0, INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, INTERNET_FLAG_DONT_CACHE);

	int iWaitTime = 20 * 60 * 1000 ;// 20分钟 
	m_cis.SetOption(INTERNET_OPTION_CONNECT_TIMEOUT, iWaitTime);		// 5000毫秒的连接超时
	m_cis.SetOption(INTERNET_OPTION_SEND_TIMEOUT, iWaitTime);			// 5000毫秒的发送超时
	m_cis.SetOption(INTERNET_OPTION_RECEIVE_TIMEOUT, iWaitTime);		// 5000毫秒的接收超时
	m_cis.SetOption(INTERNET_OPTION_DATA_SEND_TIMEOUT, iWaitTime);		// 5000毫秒的发送超时
	m_cis.SetOption(INTERNET_OPTION_DATA_RECEIVE_TIMEOUT, iWaitTime);	// 5000毫秒的接收超时
	m_cis.SetOption(INTERNET_OPTION_CONNECT_RETRIES, 1);				// 1次重试

	//	http 头
	string strHTTPBoundary = ("---------------------------7b4a6d158c9boundary");
	CString strHTTPHeader =_T("Content-Type: multipart/form-data;charset=utf-8;boundary=")+CString(strHTTPBoundary.c_str())+_T("\r\n");

	//	文件
	ULONGLONG dFL = 0,dFileSize=0;
	CString sInfo;
	CFile *fTrackEcg = new CFile(sFile.c_str(), CFile::modeRead | CFile::shareDenyWrite);
	if (fTrackEcg!=NULL)
	{
		dFileSize += fTrackEcg->GetLength();

		string sFName = sFile;
		sFName = sFName.substr(sFName.rfind('\\')+1);

		char chInfos[1024]="";
		sprintf_s(chInfos,1024,"\r\n--%s\r\nContent-Disposition: form-data; name=\"files\"; filename=\"%s\"\r\nContent-Type: application/octet-stream\r\n\r\n",strHTTPBoundary.c_str(),sFName.c_str());

		sInfo = AnsitoUtf8(chInfos).c_str();
		dFL += sInfo.GetLength();
	}

	//	参数
	CString strParamemters=_T("");
	char chInfos[502400]="";
	sprintf_s(chInfos,502400,"\r\n--%s\r\nContent-Disposition: form-data; name=\"params\"\r\nContent-Type: application/json;charset=utf-8\r\n\r\n%s\r\n",strHTTPBoundary.c_str(),Paras.c_str());
	strParamemters += AnsitoUtf8(chInfos).c_str();

	//	结尾
	CString strPostFileData=_T("\r\n--")+CString(strHTTPBoundary.c_str())+_T("--\r\n");

	//	计算内容总长度
	DWORD dwTotalRequestLength = dFL + strPostFileData.GetLength() + strParamemters.GetLength() + dFileSize;//计算整个包的总长度

	DWORD dwChunkLength = 64 * 1024;
	void* pBuffer = malloc(dwChunkLength);
	if (NULL == pBuffer) return false;
	try{
		m_pHC = m_cis.GetHttpConnection(localIp.c_str(),(INTERNET_PORT)atoi(localPort.c_str()));// IP地址和端口
		m_pHF = m_pHC->OpenRequest(CHttpConnection::HTTP_VERB_POST, uploadUrl.c_str(),NULL,NULL,NULL,NULL,INTERNET_FLAG_EXISTING_CONNECT|INTERNET_FLAG_NO_COOKIES);

		m_pHF->AddRequestHeaders(strHTTPHeader);

		if (cookies.size()>0)
			m_pHF->AddRequestHeaders(CString("Cookie: ") + cookies.c_str());	

		//	发送总长度
		m_pHF->SendRequestEx(dwTotalRequestLength, HSR_SYNC | HSR_INITIATE);

		USES_CONVERSION;

		//	读取文件
		m_pHF->Write(sInfo, sInfo.GetLength());
		DWORD dwReadLength = -1;
		while (0 != dwReadLength)
		{
			dwReadLength = fTrackEcg->Read(pBuffer, dwChunkLength);
			if (0 != dwReadLength)
				m_pHF->Write(pBuffer, dwReadLength);
		}

		//	发送附加参数
		m_pHF->Write(strParamemters, strParamemters.GetLength());

		//	发送结尾
		m_pHF->Write(strPostFileData, strPostFileData.GetLength());

		m_pHF->EndRequest(HSR_SYNC);

		DWORD dwRet;
		m_pHF->QueryInfoStatusCode(dwRet);

		int iResp = -1;
		if (dwRet == HTTP_STATUS_OK)
		{
			CString strLine;
			CString strHtml = "";
			while (m_pHF->ReadString(strLine))	{strHtml += strLine;}

			string ustr = Utf8toAnsi((char *)strHtml.GetBuffer());		
			Json::Reader reader;
			Json::Value jv;			
			if (reader.parse(ustr,jv)) //"{\"code\":1,\"message\":\"SUCCESS\",\"data\":\"OK\"}"
			{
				if (jv["code"].asInt()==1)
				{
					res = true;
				}
				else
				{
					EXCEPT("### uploadFile error : %s",jv["message"].asString().c_str());
					res = false;
				}
			}
			else
			{
				EXCEPT("### uploadFile parse json error : %s",ustr.c_str());
				res = false;
			}		
		}
		else
		{
			EXCEPT("### uploadFile Http 异常 ： %d",dwRet);
			return false;
		}		
	}
	catch (CException* e){
		TCHAR szError[MAX_PATH];
		e->GetErrorMessage(szError, MAX_PATH);
		e->Delete();
		EXCEPT("### uploadFile Error : %s",szError);
	}
	catch(exception &ee)
	{
		EXCEPT("### uploadFile Error : %s",ee.what());
	}
	catch(...)
	{
		EXCEPT("### uploadFile unKnown Error in UploadFiles!");
	}

	m_pHF->Close();

	delete m_pHF;
	delete m_pHC;
	fTrackEcg->Close();
	if (NULL != pBuffer)
		free(pBuffer);

	return res;
}

string GetJsonValuelonglong(Json::Value &jv)
{
	if (jv.isNull())
		return "";
	if (jv.isString())
		return jv.asString();

	string str = jv.toStyledString();

	if (str.find(".")!=string::npos)
		str = str.substr(0,str.find('.'));

	if (str.size()>3 && str.find(".0\n")!=string::npos)
		str = str.substr(0,str.size()-3);
	else if (str.find("\n")!=string::npos)
		str = str.substr(0,str.size()-1);

	return str;
}

vector<pWebCAMTask> MdbHttpHelper::getCAMTask(const string &sSession/*="CNC"*/,const string &smoldNo/*=""*/,const string &spartNo/*=""*/,const string &sprocessObject/*=""*/ )
{
	try
	{		
		vector<pWebCAMTask> WebTasks;
		Json::Value root;
		if (sSession=="ACAM")
			root["camType"]=2; // 1：普通CAM任务 2：校正CAM任务
		else
			root["sectionTypeName"]=sSession=="LNC"?"L":sSession;

		if (smoldNo.size()>0)
			root["moldNo"]=smoldNo;
		if (spartNo.size()>0)
			root["partNo"]=spartNo;
		if(sprocessObject.size()>0)
			root["processObject"]=sprocessObject;

		Json::FastWriter fw;
		string jParam = fw.write(root),sTxt = "";
		int iResp = httpPost(camTaskUrl,jParam,sTxt);
		if (iResp == 0 && sTxt.size()>0)
		{
			Json::Reader reader;
			Json::Value jv;			
			if (reader.parse(sTxt,jv)) 
			{
				if(jv["code"].asInt()==1)
				{
					Json::Value jc = jv["data"];
					if(jc.isArray())
					{
						for (Json::Value::iterator it = jc.begin();it!=jc.end();it++)
						{
							pWebCAMTask p = make_shared<WebCAMTask>();

							p->asmFlag = (*it)["asmFlag"].asInt();							

							p->moldNo = (*it)["moldNo"].asString();	
							p->partNo = (*it)["partNo"].asString();	
							p->processObject = (*it)["processObject"].asString();	
							p->toolingNo = (*it)["workOrderNo"].asString();								
							p->processName = (*it)["processName"].asString();	
							p->phaseName = (*it)["phaseName"].asString();	

							if (p->phaseName=="A")
								p->camStep = "T";
							else
								p->camStep = p->phaseName;

							p->section = (*it)["section"].asString();	
							p->sectionTypeName = (*it).get("sectionTypeName","").asString();	
							p->taskStatus = (*it)["taskStatus"].asInt();	
							p->taskId = (*it)["taskId"].isString()?(*it)["taskId"].asString():"";	
							p->material = (*it)["materialName"].asString();	

							p->schMfCamId = GetJsonValuelonglong((*it)["camScheduleId"]);
							p->taskId = GetJsonValuelonglong((*it)["taskId"]);
							p->moldId = GetJsonValuelonglong((*it)["moldId"]);
							p->partId = GetJsonValuelonglong((*it)["partId"]);
							p->fileIdBomPart = GetJsonValuelonglong((*it)["fileIdBomPart"]);
							p->year = (*it)["yearQuarter"].asString();	//yearQuarter=2018

							if((*it)["eleGap"].isArray())
							{
								for (Json::Value::iterator ij = (*it)["eleGap"].begin();ij!=(*it)["eleGap"].end();ij++)
								{
									if((*ij)["count"].asInt()>0 && fabs((*ij)["gap"].asDouble())>0.0001)
									{
										peleGap e = make_shared<eleGap>();
										e->count = (*ij)["count"].asInt();
										e->sparkType = (*ij)["sparkType"].asString();
										e->gap = (*ij)["gap"].asDouble();
										p->gaps.push_back(e);
									}
								}
							}
							///////////////////////////////////////////
							if((*it)["fileIdCamUploadList"].isArray())
							{
								for (Json::Value::iterator ij = (*it)["fileIdCamUploadList"].begin();ij!=(*it)["fileIdCamUploadList"].end();ij++)
								{
									string sl = (*ij)["stepName"].asString();
									string sr = GetJsonValuelonglong((*ij)["fileIdCamUpload"]);
									p->fileIdCamUploadList[sl]=sr;
								}
							}
							WebTasks.push_back(p);
						}
						return WebTasks;
					}
					else
					{
						EXCEPT("### getCAMTask : 参数错误， data 应该为数组： %s",sTxt.c_str());
					}
				}
				else
				{
					string sMsg = jv["message"].asString() + "\n" + jv["data"].asString();
					EXCEPT("### getCAMTask : %s",sMsg.c_str());
				}
			}
			else
			{
				EXCEPT("### getCAMTask : %s",sTxt.c_str());
			}
		}
	}
	catch (exception e)
	{
		EXCEPT("### getCAMTask 异常 : %s",e.what());
	}	
}

map<string,string> MdbHttpHelper::uploadNCInfo(string &para)
{
	string result = "";
	map<string,string> mRest;
	if(httpPost(cncUploadUrl,para,result)==0)
	{
		Json::Value jr;
		Json::Reader rd;
		if (rd.parse(result,jr))
		{
			if (jr["code"].asInt()==1)
			{
				string RID = GetJsonValuelonglong(jr["data"]["R"]);
				string FID = GetJsonValuelonglong(jr["data"]["F"]);
				string SID = GetJsonValuelonglong(jr["data"]["S"]);
				
				if (RID.size()>0)
					mRest["R"]=RID;
				if (SID.size()>0)
					mRest["S"]=SID;
				if (FID.size()>0)
					mRest["F"]=FID;

				return mRest;
			}
			else
			{
				string sMsg = jr["message"].asString();
				EXCEPT("### uploadNCInfo 失败 ： %s",sMsg.c_str());
			}
		}
		else
		{
			EXCEPT("### uploadNCInfo 失败 ： %s",result.c_str());
		}
	}
}

std::string MdbHttpHelper::getJsessionId()
{
	return jsessionId.substr(jsessionId.rfind('=')+1);
}

bool MdbHttpHelper::isTest()
{
	return ip != "www.moldyun.com";
}

bool MdbHttpHelper::finishTask( pWebCAMTask myTask )
{
	Json::Value jObj;
	jObj["camScheduleIds"]=myTask->schMfCamId;
	string result="";
	if(httpPost(camFinishUrl,Json::FastWriter().write(jObj),result)==0)
	{
		Json::Value jr;
		Json::Reader rd;
		if (rd.parse(result,jr))
		{
			if (jr["code"].asInt()==1)
			{
				return true;
			}
		}
	}
	EXCEPT("### finishTask 失败 ： %s",result.c_str());
}

bool MdbHttpHelper::isExistNCTemplate(vector<pCncTemplate> &vCncTempaltes,pCncTemplate t)
{
	if (vCncTempaltes.size()<1)
		return false;
	for each(pCncTemplate p in vCncTempaltes)
	{
		if(p->typeId == t->typeId)
			return true;
	}
	return false;
}

vector<pCncTemplate> MdbHttpHelper::getNCTempaltes(pWebCAMTask myTask,string sectionType)
{
	vector<pCncTemplate> vCncTempaltes;
	Json::Value jo;
	jo["moldNo"]=myTask->moldNo;
	jo["partNo"]=myTask->partNo;
	jo["processObject"]=myTask->processObject;
	string sResult="";
	int iResp = httpPost(cncTemplateUrl,Json::FastWriter().write(jo),sResult);
	if (iResp == 0)
	{
		Json::Value jr;
		Json::Reader rd;
		if (rd.parse(sResult,jr))
		{
			if (jr["code"]==1)
			{
				for (Json::ValueIterator it = jr["data"].begin();it!=jr["data"].end();it++)
				{
					pCncTemplate pCnc = make_shared<CncTemplate>();
					pCnc->typeName = (*it)["cncProgramTypeName"].asString();
					pCnc->templateFile = (*it)["prg"].asString();
					pCnc->typeId = GetJsonValuelonglong((*it)["tCamProgramTypeId"]);//(*it)["tCamProgramTypeId"].asInt();
					string sExt = (*it)["fileSuffix"].asString();
					if (sExt[0]=='.')
						sExt = sExt.substr(1);
					pCnc->fileSuffix = sExt;  // 防止 用户 配置 多了个 点
					pCnc->postName = (*it)["exePostName"].asString();
					if (sectionType.size()>0)
					{
						string st = (*it).get("sectionType","").asString();
						if (st.size()<1 || st == sectionType)
						{
							if (!isExistNCTemplate(vCncTempaltes,pCnc))
								vCncTempaltes.push_back(pCnc);
						}
					}
					else
					{
						if (!isExistNCTemplate(vCncTempaltes,pCnc))
							vCncTempaltes.push_back(pCnc);
					}
				}
				return vCncTempaltes;
			}
			else
			{
				EXCEPT("### 获取程式类型失败 ： %s",jr["message"].asString().c_str());
			}
		}
		else
		{
			EXCEPT("### 获取程式类型失败 ： %s",sResult.c_str());
		}
	}
	EXCEPT("### 获取程式类型失败!");
}

map<string,string> MdbHttpHelper::getPartFiles(string partId)
{
	map<string,string> mRest;
	Json::Value jo;
	jo["partId"]=partId;
	string sResult="";
	int iResp = httpPost(getPartFileUrl,Json::FastWriter().write(jo),sResult);
	if (iResp == 0)
	{
		Json::Value jr;
		Json::Reader rd;
		if (rd.parse(sResult,jr))
		{
			if (jr["code"]==1)
			{
				Json::Value bomPartStpList = jr["data"]["bomPartStpList"];
				if (!bomPartStpList.empty() && bomPartStpList.isArray())
				{
					for (Json::ValueIterator it = bomPartStpList.begin();it!=bomPartStpList.end();it++)
					{
						string fileName = (*it)["fileName"].asString();
						string fileId = (*it)["fileId"].asString();
						if (fileName.size()>0 && fileId.size()>1)
							mRest[fileName]=fileId;
					}					
				}				
			}
		}
	}
	return mRest;
}

