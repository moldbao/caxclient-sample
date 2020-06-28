#pragma once

#include "stdafx.h"

void MDBAPI LOG(char *format, ...);

void MDBAPI EXCEPT(char *format,...);

void MDBAPI STATUS(char *format, ...);

void MDBAPI MESG(char *format, ...);

void MDBAPI ECHO(char *format, ...);

typedef void(*LogFun)(char *s);
typedef void(*MsgFun)(char *s);

extern MDBAPI LogFun pLog,pStatus,pEcho,pExcept;
extern MDBAPI MsgFun pMsg;

string MDBAPI GetCurrentDir();

string MDBAPI GetCommonConfigPath();

void MDBAPI glogInit(const char *s="myLog");

wstring MDBAPI AnsiToUnicode(const string &szAnsi);

wstring MDBAPI AnsiToUnicode(const char *szAnsi);

string MDBAPI UnicodeToAnsi(const wstring &wszString);

string MDBAPI UnicodeToAnsi(const wchar_t *wszString);

string MDBAPI UnicodetoUtf8(const wstring &strPostData);

string MDBAPI UnicodetoUtf8(const wchar_t *strPostData);

wstring MDBAPI Utf8toUnicode(const string &strRawResponse) ;

wstring MDBAPI Utf8toUnicode(const char *strRawResponse);

string MDBAPI AnsitoUtf8(const string &sAnsi);

string MDBAPI AnsitoUtf8(const char *sAnsi);

string MDBAPI Utf8toAnsi(const string &sUtf8);

string MDBAPI Utf8toAnsi(const char *sUtf8);

string MDBAPI UrlEncode(const string &s);

string MDBAPI UrlEncode(const char *s);

string MDBAPI UrlDecode(const char *s);

string MDBAPI UrlDecode(const string &s);

// Win10 中 指针 为 long long 类型 不能使用 int | BlockUI 只能存 int | bool | string | 故 使用 string 保存 longlong 数据类型
string MDBAPI lltoString(long long t);

// Win10 中 指针 为 long long 类型 不能使用 int | BlockUI 只能存 int | bool | string | 故 使用 string 保存 longlong 数据类型
long long MDBAPI stringtoll(string s);

//	20200624105532
string MDBAPI GetCurrentDateTimeString();

//	20200624105532256
string MDBAPI GetCurrentDateTimemsString();

//	2020-06-24 10:55:32 256
string MDBAPI GetCurrentDateTimems();

//	2020-06-24 10:55:32
string MDBAPI GetCurrentDateTime();

//	2020/06/24 10:55:32
string MDBAPI GetCurrentDateTimeforMit();

//	10:55:32
string MDBAPI GetCurrentTimeOnly();

//	10:55:32 256
string MDBAPI GetCurrentTimemsOnly();

// 2020-06-24
string MDBAPI GetCurrentDate();

// 获取 系统的 版本 Windows NT 6.1
string MDBAPI getOsInfo(); 


//	int -> string
string toString(int i);

//	double -> string
string toString(double d);

//	执行 外部 exe
void ProcessCmd(const char *cExe,const char *cPara,bool showWindow=false);
