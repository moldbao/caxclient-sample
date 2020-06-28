#pragma once

#include "stdafx.h"

#include <vector>
#include <string>
#include <sstream>
using namespace std;

bool MDBAPI ZipFiles(vector<string> &vFiles,const string &sZipName);

bool MDBAPI ZipFiles(string &sFile,const string &sZipName);

bool MDBAPI UnZipFiles(const string &szip,const string &sSaveFolder);

/*
	vector<string> vStr;
	vStr.push_back("D:\\MDB_ROOT\\EDM\\C2009\\9-PM09110-101\\E003\\EDM3_R\\9-PM09110-101E001_CF0305.PRT");
	vStr.push_back("D:\\MDB_ROOT\\EDM\\C2009\\9-PM09110-101\\E003\\EDM3_R\\9-PM09110-101E001_CF0306.PRT");
	vStr.push_back("D:\\MDB_ROOT\\EDM\\C2009\\9-PM09110-101\\E003\\EDM3_R\\9-PM09110-101E003_CF0301.PRT");
	vStr.push_back("D:\\MDB_ROOT\\EDM\\C2009\\9-PM09110-101\\E003\\EDM3_R\\9-PM09110-101E003_CF0302.PRT");

	if(ZipFiles(vStr,"D:\\abc.zip"))
	{
		printf("zip OK!\n");
		if(UnZipFiles("D:\\abc.zip","D:\\123567"))
			printf("unzip ok!\n");
		else
			printf("unzip ng!\n");
	}
	else
		printf("zip NG!\n");
*/