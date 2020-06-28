#include "MdbCommon.h"

#include <atlbase.h>
#include <io.h>

#define MAXSTRLENTH (50000)

LogFun pLog=NULL,pStatus=NULL,pEcho=NULL,pExcept=NULL;
MsgFun pMsg=NULL;

void AddMsgEnd(char *msg)
{
	string sEnd = "...[string is too long, Please use Fiddler to catch this log]\n";
	long ll = strlen(msg)-sEnd.size();
	for (int i=0;i<sEnd.size();i++)
	{
		msg[ll+i]=sEnd[i];
	}
}

//	打印信息至日志
void LOG(char *format, ...)
{
	char msg[MAXSTRLENTH+1];
	va_list args;
	va_start(args, format);
	int nsize = vsnprintf_s(msg, sizeof(msg), _TRUNCATE, format, args);
	va_end(args);

	if (pLog==NULL)
		nsize = _snprintf_s(msg, sizeof(msg),  _TRUNCATE,"%s", ("[" + GetCurrentDateTimems() + "] " + msg).c_str()); // 增加 时间

	if (msg[strlen(msg)-1]!='\n')
	{
		if (nsize == -1) // 数据 已经 越界了
			AddMsgEnd(msg);
		else
			strcat_s(msg,MAXSTRLENTH,"\n");
	}

	if (pLog!=NULL)
		pLog(msg);
	else
		printf("%s",msg);
}

//	抛出 异常 exception ： 格式化异常信息
void EXCEPT(char *format,...)
{
	char msg[MAXSTRLENTH+1];
	va_list args;
	va_start(args, format);
	int nsize = vsnprintf_s(msg, sizeof(msg), _TRUNCATE, format, args);
	va_end(args);

	if (msg[strlen(msg)-1]!='\n')
	{
		if (nsize == -1) // 数据 已经 越界了
			AddMsgEnd(msg);
		else
			strcat_s(msg,MAXSTRLENTH,"\n");
	}

	if (pExcept!=NULL)
		pExcept(msg);
	else
		printf("%s",msg);

	throw exception(msg);
}

//	显示 状态 信息
void STATUS(char *format, ...)
{
	char msg[132+1];
	va_list args;
	va_start(args, format);
	int nsize = vsnprintf_s(msg, sizeof(msg), _TRUNCATE, format, args);
	va_end(args);

	if (pStatus!=NULL)
	{
		printf("%s",msg);
		if (msg[strlen(msg)-1]!='\n')
		{
			if (nsize == -1) // 数据 已经 越界了
				AddMsgEnd(msg);
			else
				strcat_s(msg,132,"\n");
		}
		pStatus(msg);
	}
	else
		printf("%s",msg);
}

//	显示 消息 uc1601
void MESG(char *format, ...)
{
	char msg[132+1];
	va_list args;
	va_start(args, format);
	vsnprintf_s(msg, sizeof(msg), _TRUNCATE, format, args);
	va_end(args);

	if (pMsg!=NULL)
		pMsg(msg);
	else
		printf("%s",msg);
}

//	打印信息之信息窗口
void ECHO(char *format, ...)
{
	char msg[MAXSTRLENTH+1];
	va_list args;
	va_start(args, format);
	int nsize = vsnprintf_s(msg, sizeof(msg), _TRUNCATE, format, args);
	va_end(args);

	if (msg[strlen(msg)-1]!='\n')
	{
		if (nsize == -1) // 数据 已经 越界了
			AddMsgEnd(msg);
		else
			strcat_s(msg,MAXSTRLENTH,"\n");
	}

	if (pEcho!=NULL)
	{
		printf("%s",msg);
		pEcho(msg);
	}
	else
		printf("%s",msg);
}

//	获取 当前的 exe/dll 运行目录
string GetCurrentDir()
{
	char	szBuff[MAX_PATH] = {0};
	HMODULE hModuleInstance = _AtlBaseModule.GetModuleInstance();
	GetModuleFileNameA(hModuleInstance,szBuff, MAX_PATH);
	string sFP = szBuff;
	return sFP.substr(0,sFP.rfind("\\"));
}

string GetCommonConfigPath()
{
	string sCurPath = GetCurrentDir();
	string str = sCurPath + "\\MDB.Common.dll.config";
	if (_access(str.c_str(),0)==-1)
	{
		str = "T:\\NXCustomize_x64\\MOLDBAO_x64\\MDB.Common.dll.config";
		if (_access(str.c_str(),0)==-1)
			str = "T:\\NXCustomize_win32\\MOLDBAO_win32\\MDB.Common.dll.config";
	}
	return str;
}

wstring AnsiToUnicode(const string &szAnsi)
{
	int wcsLen = ::MultiByteToWideChar(CP_ACP, NULL, szAnsi.c_str(), szAnsi.size(), NULL, 0);  
	wchar_t* wszString = new wchar_t[wcsLen + 1];  
	::MultiByteToWideChar(CP_ACP, NULL, szAnsi.c_str(), szAnsi.size(), wszString, wcsLen);  
	wszString[wcsLen] = '\0';  
	wstring wstr = wszString;
	delete[] wszString;
	return wstr;
}

wstring AnsiToUnicode(const char *szAnsi){return AnsiToUnicode(string(szAnsi));}

string UnicodeToAnsi(const wstring &wszString)
{
	int ansiLen = ::WideCharToMultiByte(CP_ACP, NULL, wszString.c_str(), wszString.size(), NULL, 0, NULL, NULL);  
	char* szAnsi = new char[ansiLen + 1];  
	::WideCharToMultiByte(CP_ACP, NULL, wszString.c_str(), wszString.size(), szAnsi, ansiLen, NULL, NULL);  
	szAnsi[ansiLen] = '\0'; 
	string str = szAnsi;
	delete[] szAnsi;
	return str;
}
string UnicodeToAnsi(const wchar_t *wszString){return UnicodeToAnsi(wstring(wszString));}


string UnicodetoUtf8(const wstring &strPostData)
{
	string str="";
	DWORD  num = WideCharToMultiByte(CP_UTF8, 0, strPostData.c_str(), -1, NULL, 0, NULL, NULL);//計算這個UNICODE实际由几个UTF-8字組成
	char*cword = new char[num +1];
	WideCharToMultiByte(CP_UTF8, 0, strPostData.c_str(), -1, cword, num, NULL, NULL);
	str = cword;
	delete cword;
	return str;
}

string UnicodetoUtf8(const wchar_t *strPostData){	return UnicodetoUtf8(wstring(strPostData));}


wstring Utf8toUnicode(const string &strRawResponse) 
{
	wstring wstr=L"";
	int unicodeLen = MultiByteToWideChar(CP_UTF8, 0, strRawResponse.c_str(), -1, NULL, 0);
	WCHAR *pUnicode = new WCHAR[unicodeLen + 1];
	memset(pUnicode, 0, (unicodeLen + 1)*sizeof(wchar_t));
	MultiByteToWideChar(CP_UTF8, 0, strRawResponse.c_str(), -1, pUnicode, unicodeLen);
	wstr = pUnicode;
	delete pUnicode;
	return wstr;
}

wstring Utf8toUnicode(const char *strRawResponse) {	return Utf8toUnicode(string(strRawResponse));}

string AnsitoUtf8(const string &sAnsi)
{
	wstring wUnicode = AnsiToUnicode(sAnsi);
	return UnicodetoUtf8(wUnicode);
}

string AnsitoUtf8(const char *sAnsi){return AnsitoUtf8(string(sAnsi));}

string Utf8toAnsi(const string &sUtf8)
{
	wstring sUnicode = Utf8toUnicode(sUtf8);
	return UnicodeToAnsi(sUnicode);
}

string Utf8toAnsi(const char *sUtf8){return Utf8toAnsi(string(sUtf8));}

//	url 编码 解码
class strCoding{
	void Gb2312ToUnicode(WCHAR *pOut, char *gbBuffer){
		::MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, gbBuffer, 2, pOut, 1);
	}
	void UTF_8ToUnicode(WCHAR *pOut, char *pText)	{
		char *uchar = (char *)pOut;
		uchar[1] = ((pText[0] & 0x0F) << 4) + ((pText[1] >> 2) & 0x0F);
		uchar[0] = ((pText[1] & 0x03) << 6) + (pText[2] & 0x3F);
	}
	void UnicodeToUTF_8(char *pOut, WCHAR *pText)	{
		char *pchar = (char *)pText;
		pOut[0] = (0xE0 | ((pchar[1] & 0xF0) >> 4));
		pOut[1] = (0x80 | ((pchar[1] & 0x0F) << 2)) + ((pchar[0] & 0xC0) >> 6);
		pOut[2] = (0x80 | (pchar[0] & 0x3F));
		return;
	}
	void UnicodeToGB2312(char *pOut, WCHAR uData)	{
		WideCharToMultiByte(CP_ACP, NULL, &uData, 1, pOut, sizeof(WCHAR), NULL, NULL);
	}
	char CharToInt(char ch)	{
		if(ch >= '0' && ch <= '9')return (char)(ch - '0');
		if(ch >= 'a' && ch <= 'f')return (char)(ch - 'a' + 10);
		if(ch >= 'A' && ch <= 'F')return (char)(ch - 'A' + 10);
		return -1;
	}
	char StrToBin(char *str){
		char tempWord[2];
		char chn;
		tempWord[0] = CharToInt(str[0]);    // make the B to 11 -- 00001011
		tempWord[1] = CharToInt(str[1]);    // make the 0 to 0 -- 00000000
		chn = (tempWord[0] << 4) | tempWord[1];    // to change the BO to 10110000
		return chn;
	}
	// UTF_8 转 gb2312
	void UTF_8ToGB2312(string &pOut, char *pText, int pLen)	{
		char buf[4];
		char *rst = new char[pLen + (pLen >> 2) + 2];
		memset(buf, 0, 4);
		memset(rst, 0, pLen + (pLen >> 2) + 2);
		int i =0;
		int j = 0;
		while(i < pLen)
		{
			if(*(pText + i) >= 0)
				rst[j++] = pText[i++];
			else{
				WCHAR Wtemp;
				UTF_8ToUnicode(&Wtemp, pText + i);
				UnicodeToGB2312(buf, Wtemp);
				unsigned short int tmp = 0;
				tmp = rst[j] = buf[0];
				tmp = rst[j + 1] = buf[1];
				tmp = rst[j + 2] = buf[2];
				i += 3;
				j += 2;
			}
		}
		rst[j] = '\0';
		pOut = rst;
		delete []rst;
	}
	// GB2312 转 UTF-8
	void GB2312ToUTF_8(string& pOut, char *pText, int pLen)	{
		char buf[4];
		memset(buf, 0, 4);
		pOut.clear();
		int i = 0;
		while(i < pLen)	{
			// 如果是英文直接复制就可以
			if( pText[i] >= 0){
				char asciistr[2] = {0};
				asciistr[0] = (pText[i++]);
				pOut.append(asciistr);
			}
			else{
				WCHAR pbuffer;
				Gb2312ToUnicode(&pbuffer, pText + i);
				UnicodeToUTF_8(buf,&pbuffer);
				pOut.append(buf);
				i += 2;
			}
		}
		return;
	}
	// 把str编码为网页中的 GB2312 url encode ,英文不变，汉字双字节 如%3D%AE%88
	string UrlGB2312(char *str){
		string dd;
		size_t len = strlen(str);
		for (size_t i = 0; i < len; i++){
			if(isalnum((BYTE)str[i])){
				char tempbuff[2];
				sprintf_s(tempbuff, 2,"%c", str[i]);
				dd.append(tempbuff);
			}
			else if (isspace((BYTE)str[i]))
				dd.append("+");
			else{
				char tempbuff[4];
				sprintf_s(tempbuff,4, "%%%X%X", ((BYTE*)str)[i] >>4, ((BYTE*)str)[i] % 16);
				dd.append(tempbuff);
			}
		}
		return dd;
	}
	// 把url GB2312解码
	string UrlGB2312Decode(string str){
		string output = "";
		char tmp[2];
		int i = 0, idx = 0, ndx, len = str.length();
		while(i<len){
			if(str[i] == '%'){
				tmp[0] = str[i + 1];
				tmp[1] = str[i + 2];
				output += StrToBin(tmp);
				i = i + 3;
			}
			else if(str[i] == '+'){
				output += ' ';
				i++;
			}
			else{
				output += str[i];
				i++;
			}
		}
		return output;
	}	
public:
	strCoding(void){};
	~strCoding(void){};
	// 把str编码为网页中的 UTF-8 url encode ,英文不变，汉字三字节 如%3D%AE%88
	string UrlUTF8(string &str){
		string tt;
		string dd;
		GB2312ToUTF_8(tt, (char *)str.c_str(), str.size());
		size_t len=tt.length();
		for (size_t i = 0; i < len; i++)
		{
			if(isalnum((BYTE)tt.at(i)))	{
				char tempbuff[2] = {0};
				sprintf_s(tempbuff, 2,"%c", (BYTE)tt.at(i));
				dd.append(tempbuff);
			}
			else if (isspace((BYTE)tt.at(i)))
				dd.append("+");
			else{
				char tempbuff[4];
				sprintf_s(tempbuff,4, "%%%X%X", ((BYTE)tt.at(i)) >> 4, ((BYTE)tt.at(i)) % 16);
				dd.append(tempbuff);
			}
		}
		return dd;
	}
	// 把url utf8解码
	string UrlUTF8Decode(string str){
		string output = "";
		string temp = UrlGB2312Decode(str);
		UTF_8ToGB2312(output, (char *)temp.data(), strlen(temp.data()));
		return output;
	}
};

string UrlEncode(const string &s)
{
	strCoding sc;
	return sc.UrlUTF8(string(s));
}

string UrlEncode(const char *s)
{
	strCoding sc;
	return sc.UrlUTF8(string(s));
}

string UrlDecode(const char *s)
{
	strCoding sc;
	return sc.UrlUTF8Decode(string(s));
}

string UrlDecode(const string &s)
{
	strCoding sc;
	return sc.UrlUTF8Decode(string(s));
}

// Win10 中 指针 为 long long 类型 不能使用 int | BlockUI 只能存 int | bool | string | 故 使用 string 保存 longlong 数据类型
string lltoString(long long t)
{
	string result;
	stringstream ss;
	ss <<  t;
	ss >> result;
	//LOG(">>> lltoString : %lld : %s",t,result.c_str());
	return result;
}

// Win10 中 指针 为 long long 类型 不能使用 int | BlockUI 只能存 int | bool | string | 故 使用 string 保存 longlong 数据类型
long long stringtoll(string s) 
{
	long long ll;
	stringstream ss;
	ss << s;
	ss >> ll;
	//LOG(">>> stringtoll : %s : %lld",s.c_str(),ll);
	return ll;
}

//	20200624105532
string GetCurrentDateTimeString()
{
	SYSTEMTIME st = {0};
	GetLocalTime(&st);
	char chs[256]="";
	_snprintf_s(chs,256,"%d%02d%02d%02d%02d%02d",
		st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond);
	return chs;
}

//	20200624105532256
string GetCurrentDateTimemsString()
{
	SYSTEMTIME st = {0};
	GetLocalTime(&st);
	char chs[256]="";
	_snprintf_s(chs,256,"%d%02d%02d%02d%02d%02d%03d",
		st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond,st.wMilliseconds);
	return chs;
}

//	2020-06-24 10:55:32 256
string GetCurrentDateTimems()
{
	SYSTEMTIME st = {0};
	GetLocalTime(&st);
	char chs[256]="";
	_snprintf_s(chs,256,"%d-%02d-%02d %02d:%02d:%02d %03d",
		st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond,st.wMilliseconds);
	return chs;
}

//	2020-06-24 10:55:32
string GetCurrentDateTime()
{
	SYSTEMTIME st = {0};
	GetLocalTime(&st);
	char chs[256]="";
	_snprintf_s(chs,256,"%d-%02d-%02d %02d:%02d:%02d",
		st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond);
	return chs;
}

//	2020/06/24 10:55:32
string GetCurrentDateTimeforMit()
{
	SYSTEMTIME st = {0};
	GetLocalTime(&st);
	char chs[256]="";
	_snprintf_s(chs,256,"%d/%02d/%02d %02d:%02d:%02d",
		st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond);
	return chs;
}

//	10:55:32
string GetCurrentTimeOnly()
{
	SYSTEMTIME st = {0};
	GetLocalTime(&st);
	char chs[256]="";
	_snprintf_s(chs,256,"%02d:%02d:%02d",st.wHour,st.wMinute,st.wSecond);
	return chs;
}

//	10:55:32 256
string GetCurrentTimemsOnly()
{
	SYSTEMTIME st = {0};
	GetLocalTime(&st);
	char chs[256]="";
	_snprintf_s(chs,256,"%02d:%02d:%02d %03d",st.wHour,st.wMinute,st.wSecond,st.wMilliseconds);
	return chs;
}

// 2020-06-24
string GetCurrentDate()
{
	SYSTEMTIME st = {0};
	GetLocalTime(&st);
	char chs[256]="";
	_snprintf_s(chs,256,"%d-%02d-%02d",st.wYear,st.wMonth,st.wDay);
	return chs;
}

string getOsInfo()  // 获取 系统的 版本 Windows NT 6.1
{
	typedef void(__stdcall*NTPROC)(DWORD*, DWORD*, DWORD*);
	HINSTANCE hinst = LoadLibrary("ntdll.dll");
	DWORD dwMajor, dwMinor, dwBuildNumber;
	NTPROC proc = (NTPROC)GetProcAddress(hinst, "RtlGetNtVersionNumbers"); 
	proc(&dwMajor, &dwMinor, &dwBuildNumber); 
	if (dwMajor == 6 && dwMinor == 3)	//win 8.1
		return "Windows NT 8.1";
	if (dwMajor == 10 && dwMinor == 0)	//win 10
		return "Windows NT 10.0";

	OSVERSIONINFO osver = { sizeof(OSVERSIONINFO) };
	GetVersionEx(&osver);
	stringstream ss;
	ss << "Windows NT " << osver.dwMajorVersion << '.' << osver.dwMinorVersion;
	return ss.str();
}

//	int -> string
string toString(int i)
{
	char chs[256]="";
	_snprintf_s(chs,256,"%d",i);
	return chs;
}

//	double -> string
string toString(double d)
{
	if (fabs(d)<0.0001)
		return "0";

	char chs[256]="";
	_snprintf_s(chs,256,"%.3f",d);
	if (strstr(chs,".000")!=NULL)
		_snprintf_s(chs,256,"%g",d);
	string sOut = chs;
	if (sOut.find('.')!=string::npos)
	{
		while(sOut.size()>1 && sOut[sOut.size()-1]=='0') // 去除末尾的0
		{
			sOut = sOut.substr(0,sOut.rfind('0'));
		}
	}
	return sOut;
}

void ProcessCmd(const char *cExe,const char *cPara,bool showWindow)
{
	std::string CMMD=string(cExe) + " " + cPara;

	STARTUPINFO si = { sizeof(si) };   

	PROCESS_INFORMATION pi;   
	si.dwFlags = STARTF_USESHOWWINDOW;   
	si.wShowWindow = showWindow?TRUE:FALSE; //TRUE表示显示创建的进程的窗口  	
	char cmdline[502400];
	strcpy_s(cmdline,502400,(char *)CMMD.data());
	LOG(">>> ProcessCmd : %s",cmdline);
	try
	{
		USES_CONVERSION;
		BOOL bRet = ::CreateProcess (NULL, A2T(cmdline),NULL,NULL,FALSE,CREATE_NEW_CONSOLE,NULL,NULL,&si,&pi);
		DWORD error = ::GetLastError();  
		if(bRet) 
		{
			while(WaitForSingleObject(pi.hProcess, INFINITE)==WAIT_OBJECT_0)  // 会阻塞主进程 等待 进程执行完毕
			{
				::CloseHandle (pi.hThread); 
				::CloseHandle (pi.hProcess); 
			}
		}
		LOG(">>> ProcessCmd END!");
	}
	catch (...)
	{
		LOG("### ProcessCmd END!");
	}	
}
