#pragma once

#include "stdafx.h"
#include "MdbCommon.h"

struct MDBAPI eleGap
{
	string sparkType;
	double gap;
	int count;
};
typedef shared_ptr<eleGap> peleGap; 

struct MDBAPI asmPart
{
	string fileId;
	string moldNo;
	string partNo;
	string processObject;

	string toString()
	{
		stringstream ss;
		ss << "moldNo=" << moldNo << " partNo="<<partNo<<" processObject="<<processObject<<" fileId="<<fileId;
		return ss.str();
	}
};
typedef shared_ptr<asmPart> pasmPart; 

//	CAM 任务
struct MDBAPI WebCAMTask
{
	string moldNo;
	string partNo;
	string processObject;
	string material;
	string toolingNo;
	string processName;
	string phaseName;
	string section;
	string sectionTypeName;
	int taskStatus;
	bool isUploaded;
	string gapHrc;
	string dwgNo;
	string schMfCamId;
	string schMfCamProcessId;
	string taskId;
	string moldId;
	string partId;
	string zipFile;
	string fileName;
	string filePath;
	string fileIdBomPart;
	string file;
	vector<peleGap> gaps;
	vector<pasmPart> asmList;
	int asmFlag;
	map<string,string> fileIdCamUploadList;

	string camStep;			//	M4 工部

	string year;			//	mit 年份 + M4 年份
	string sRGap;			//	mit R Gap
	string sFGap;			//	mit F Gap
	string sXlsPath;		//	mit xls 路径
	string sXlsExcel;		//	mit xls 文件名
	string sXls;			//	mit xls 完成路径
	bool isMit;

	WebCAMTask(){
		isMit = false;
	};
	WebCAMTask(string m,string p)
	{
		isMit = false;
		moldNo = m;
		partNo = p;
		processName = "EDM";
		material = "钢件";
		toolingNo = "-";
		phaseName = "A";
		section = "EDM";
		processObject ="WORKPIECE";
	}

	string getGapString()
	{
		stringstream ss;
		for each (peleGap e in gaps)
		{
			ss << e->sparkType + ":"+toString(e->gap) + "|";
		}
		string sg = ss.str();
		return sg.substr(0,sg.size()-1);
	}


	string getGapStringforMit() // mit
	{
		if (sRGap=="NA")
			sRGap="";
		if (sFGap=="NA")
			sFGap = "";

		int iRF = 1;
		if (sRGap.size()>0 && sFGap.size()>0)
			iRF = 2;

		if (iRF==2)
		{
			return "F:" + sFGap + "|" + "R:"+sRGap;
		}
		else
		{
			if (sFGap.size()>0)
				return "F:"+sFGap;
			else
				return "R:"+sRGap;
		}		
	}
	string getGapStringforMit2() // mit
	{
		if (sRGap=="NA")
			sRGap="";
		if (sFGap=="NA")
			sFGap = "";

		int iRF = 1;
		if (sRGap.size()>0 && sFGap.size()>0)
			iRF = 2;

		if (iRF==2)
		{
			return "R-"+sRGap+","+"F-" + sFGap;
		}
		else
		{
			if (sFGap.size()>0)
				return ",F-"+sFGap;
			else
				return "R-"+sRGap+",";
		}		
	}
};

typedef shared_ptr<WebCAMTask> pWebCAMTask; // 【智能指针】 电极信息


struct MDBAPI CncTemplate
{
	string typeName;
	string templateFile;
	string fileSuffix;
	string postName;
	string typeId;
};
typedef shared_ptr<CncTemplate> pCncTemplate; 

