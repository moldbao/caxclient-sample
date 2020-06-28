#pragma once
#include "stdafx.h"
#include <afxole.h> 
#include <string>
#include <vector>
#include <map>
using namespace std;
class MDBAPI MdbConstants
{
public:
	MdbConstants(void);
	~MdbConstants(void);
	CMapStringToString constantsMap;
	CString FILE_ROOT;
	CString WebSystemURL;
	CString FILE_ROOT_HTTP;
	int MOLD_NO_LEN;
	CString LOCAL_ROOT;
	CString TempDir;
	CString NX_RES_PATH;
};