// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>

#ifdef _MDBOPENAPI_EXPORT_
#define MDBAPI __declspec(dllexport)
#else
#define MDBAPI __declspec(dllimport)
#endif // _MDBOPENAPI_EXPORT_

#include <stdarg.h>
#include <io.h>
#include <direct.h>

#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <memory>
#include <string>
#include <sstream>
#include <fstream>
using namespace std;

// TODO: reference additional headers your program requires here
