#include "stdafx.h"
#include "MdbConstants.h"
#include "Markup/Markup.h"

#include "MdbCommon.h"

MdbConstants::MdbConstants(void)
{
	//printf("+++ MdbConstants\n");
	string scfg = GetCommonConfigPath();
	if (scfg.size()<1 || _access(scfg.c_str(),0)==-1)
		throw exception("没有找到配置文件！");

	CMarkup xml;
	xml.Load(scfg.c_str());

	xml.ResetMainPos();   
	xml.FindElem();      // 根节点

	xml.IntoElem();
	if (xml.FindElem("appSettings"))
	{
		xml.IntoElem();

		while (xml.FindElem("add"))
		{
			string k = xml.GetAttrib("key");
			string v = xml.GetAttrib("value");

			constantsMap[k.c_str()]=v.c_str();
		}
		xml.OutOfElem();
	}
	xml.OutOfElem();

	FILE_ROOT = constantsMap["FILE_ROOT"];
	WebSystemURL = constantsMap["WebSystemURL"];
	FILE_ROOT_HTTP = constantsMap["FILE_ROOT_HTTP"];
	MOLD_NO_LEN = atoi(constantsMap["FILE_ROOT"]);
	LOCAL_ROOT = constantsMap["LOCAL_ROOT"];

	char *p = getenv("MDBLOCALROOT");
	if(p!=NULL && _access(p,0)!=-1)
		LOCAL_ROOT = CString(p);

	TempDir = constantsMap["TempDir"];
	NX_RES_PATH = constantsMap["NX_RES_PATH"];
}

MdbConstants::~MdbConstants(void)
{
	//printf("--- MdbConstants\n");
}
