#include "stdafx.h"
#include "ZipHelper.h"

#define FOPEN_FUNC(filename, mode) fopen64(filename, mode)
#define FTELLO_FUNC(stream) ftello64(stream)
#define FSEEKO_FUNC(stream, offset, origin) fseeko64(stream, offset, origin)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>

# include <direct.h>
# include <io.h>

#include "zip.h"
#include "unzip.h"

#ifndef _WIN64
#pragma comment(lib,"zlib\\lib\\x86\\zlibwapi.lib")
#else
#pragma comment(lib,"zlib\\lib\\x64\\zlibwapi.lib")
#endif // _WIN64

#ifdef _WIN32
#define USEWIN32IOAPI
#include "iowin32.h"
#endif

#define CASESENSITIVITY (0)
#define WRITEBUFFERSIZE (16384)
#define MAXFILENAME (256)

#include "MdbCommon.h"

uLong filetime(const char *f, tm_zip *tmzip, uLong *dt)
{
	int ret = 0;
	{
		FILETIME ftLocal;
		HANDLE hFind;
		WIN32_FIND_DATAA ff32;

		hFind = FindFirstFileA(f,&ff32);
		if (hFind != INVALID_HANDLE_VALUE)
		{
			FileTimeToLocalFileTime(&(ff32.ftLastWriteTime),&ftLocal);
			FileTimeToDosDateTime(&ftLocal,((LPWORD)dt)+1,((LPWORD)dt)+0);
			FindClose(hFind);
			ret = 1;
		}
	}
	return ret;
}

int check_exist_file(const char* filename)
{
	FILE* ftestexist;
	int ret = 1;
	ftestexist = FOPEN_FUNC(filename,"rb");
	if (ftestexist==NULL)
		ret = 0;
	else
		fclose(ftestexist);
	return ret;
}

void change_file_date(const char *filename,uLong dosdate,tm_unz tmu_date)
{
	HANDLE hFile;
	FILETIME ftm,ftLocal,ftCreate,ftLastAcc,ftLastWrite;

	hFile = CreateFileA(filename,GENERIC_READ | GENERIC_WRITE,
		0,NULL,OPEN_EXISTING,0,NULL);
	GetFileTime(hFile,&ftCreate,&ftLastAcc,&ftLastWrite);
	DosDateTimeToFileTime((WORD)(dosdate>>16),(WORD)dosdate,&ftLocal);
	LocalFileTimeToFileTime(&ftLocal,&ftm);
	SetFileTime(hFile,&ftm,&ftLastAcc,&ftm);
	CloseHandle(hFile);
}

int mymkdir(const char* dirname)
{
	int ret=0;
	ret = _mkdir(dirname);
	return ret;
}

int makedir(const char *newdir)
{
	char *buffer ;
	char *p;
	int  len = (int)strlen(newdir);

	if (len <= 0)
		return 0;

	buffer = (char*)malloc(len+1);
	if (buffer==NULL)
		throw new exception("make dir Error allocating memory!");

	strcpy(buffer,newdir);

	if (buffer[len-1] == '/') {
		buffer[len-1] = '\0';
	}
	if (mymkdir(buffer) == 0)
	{
		free(buffer);
		return 1;
	}

	p = buffer+1;
	while (1)
	{
		char hold;

		while(*p && *p != '\\' && *p != '/')
			p++;
		hold = *p;
		*p = 0;
		if ((mymkdir(buffer) == -1) && (errno == ENOENT))
			throw new exception("couldn't create directory");
		if (hold == 0)
			break;
		*p++ = hold;
	}
	free(buffer);
	return 1;
}

//#include "EleUtil.h"

int isLargeFile(const char* filename)
{
	//LOG(">>> isLargeFile");
	int largeFile = 0;
	ZPOS64_T pos = 0;
	FILE* pFile = FOPEN_FUNC(filename, "rb");
	if(pFile != NULL)
	{
		int n = FSEEKO_FUNC(pFile, 0, SEEK_END);
		pos = FTELLO_FUNC(pFile);
		if(pos >= 0xffffffff)
			largeFile = 1;
		fclose(pFile);
	}
	return largeFile;
}

bool isEmptyZipFile(const char* filename)
{
	ZPOS64_T pos = 0;
	FILE* pFile = FOPEN_FUNC(filename, "rb");
	if(pFile != NULL)
	{
		int n = FSEEKO_FUNC(pFile, 0, SEEK_END);
		pos = FTELLO_FUNC(pFile);
		fclose(pFile);
	}
	return pos < 45; // 小于 45 个字节的 认为是空的压缩包
}

bool ZipFiles(string &sFile,const string &sZipName)
{
	vector<string> vFiles;
	vFiles.push_back(sFile);
	return ZipFiles(vFiles,sZipName);
}

//	使用 ziplib 进行 文件压缩
bool _ZipFiles(vector<string> &vFiles,const string &sZipName)
{
	try
	{
		//LOG(">>> ZipFiles : %d | %s",vFiles.size(),sZipName.c_str());
		int opt_overwrite=1;
		int opt_compress_level=5;
		int opt_exclude_path=0;  // 1 保存完整的目录结构 0 无目录结构

		char filename_try[MAXFILENAME+16];
		int err=0;
		int size_buf=0;
		void* buf=NULL;
		const char* password=NULL;

		size_buf = WRITEBUFFERSIZE;
		buf = (void*)malloc(size_buf);
		if (buf!=NULL)
		{
			strcpy_s(filename_try,MAXFILENAME,sZipName.c_str());

			zipFile zf;
			zlib_filefunc64_def ffunc;
			//LOG(">>> fill_win32_filefunc64A");
			fill_win32_filefunc64A(&ffunc);
			//LOG(">>> zipOpen2_64");
			zf = zipOpen2_64(filename_try,(opt_overwrite==2) ? 2 : 0,NULL,&ffunc);
			if (zf!=NULL)
			{
				bool bAdd = false;
				for (int i=0;i<(int)vFiles.size();i++)
				{			
					if (_access(vFiles[i].c_str(),0)==-1)  // 文件 不存在时 跳过 ... ...
						continue;

					bAdd = true;

					FILE * fin = NULL;
					int size_read;
					const char* filenameinzip = vFiles[i].c_str();
					const char *savefilenameinzip;
					zip_fileinfo zi;
					int zip64 = 0;

					zi.tmz_date.tm_sec = zi.tmz_date.tm_min = zi.tmz_date.tm_hour =
						zi.tmz_date.tm_mday = zi.tmz_date.tm_mon = zi.tmz_date.tm_year = 0;
					zi.dosDate = 0;
					zi.internal_fa = 0;
					zi.external_fa = 0;
					//LOG(">>> filetime");
					filetime(filenameinzip,&zi.tmz_date,&zi.dosDate);			
					zip64 = isLargeFile(filenameinzip);

					string sNN = filenameinzip;
					if (opt_exclude_path == 0)
						sNN = sNN.substr(sNN.rfind('\\')+1);
					else
					{
						if (sNN[0]=='\\')
							sNN = sNN.substr(1);
						else if (sNN[1]==':')
							sNN = sNN.substr(3);
					}
					savefilenameinzip = sNN.c_str(); // 文件名 转 utf-8

					//LOG(">>> zipOpenNewFileInZip3_64");
					err = zipOpenNewFileInZip3_64(zf,savefilenameinzip,&zi,NULL,0,NULL,0,NULL,
						(opt_compress_level != 0) ? Z_DEFLATED : 0,opt_compress_level,0,		-MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
						password,0, zip64);

					if (err == ZIP_OK)
					{
						fin = FOPEN_FUNC(filenameinzip,"rb");
						if (fin!=NULL)
						{
							//LOG(">>> zipWriteInFileInZip");
							do{
								err = ZIP_OK;
								size_read = (int)fread(buf,1,size_buf,fin);								
								err = zipWriteInFileInZip (zf,buf,size_read);
								if (size_read < size_buf && feof(fin)==0)
									err = ZIP_ERRNO;
							} while ((err == ZIP_OK) && (size_read>0));
							fclose(fin);

							if (err == ZIP_OK)
								err = zipCloseFileInZip(zf);
							else
								throw new exception("make zip file error!");				
						}
					}
				}
				if (zipClose(zf,NULL)!=ZIP_OK)
					throw new exception("close zip file error!");
				free(buf);

				if (!bAdd)
					throw new exception("no file can be zipped!");

				if (isEmptyZipFile(sZipName.c_str()))
					throw new exception("zip file is empty!");
				//LOG(">>> Zip END !!!");
				return true;
			}
			else
				throw new exception("open zip file error!");
		}
		else
			throw new exception("unknow error!");
	}
	catch (...)
	{
		throw new exception(" # ZipFiles # unknow error!");
	}
	return false;	
}

bool ZipFiles(vector<string> &vFiles,const string &sZipName)
{
	stringstream ss;
	for each (string s in vFiles)
	{
		ss << "\"" + s + "\" ";
	}
	string sInfile = ss.str();

	bool bChs = false;
	for each (char c in sInfile)
	{
		if (c >=0 && c<=127)
			continue;
		else
			bChs = true;
	}

	if (bChs)
	{
		string sexe = GetCurrentDir() + "\\ZipMaker.exe"; // 调用 ZipMaker.exe 创建 压缩包 解决 文件名中 有 中文，上传后出现乱码的问题 ： 20191203
		if (_access(sexe.c_str(),0)!=-1)
		{
			LOG(">>> use zipmaker ...");
			string sCmd = "zip \"" +sZipName+"\" " + sInfile; 
			ProcessCmd(sexe.c_str(),sCmd.c_str());
			if (_access(sZipName.c_str(),0)!=-1)
				return true;
			else
				LOG("### use zipmaker make zip NG!");
		}
	}

	return _ZipFiles(vFiles,sZipName);
}

//////////////////////////////////////////////////////////////////////////
//	zip 解压

int do_extract_currentfile(unzFile uf,const int*popt_extract_without_path,const char*password)
{
	char filename_inzip[256];
	char* filename_withoutpath;
	char* p;
	int err=UNZ_OK;
	FILE *fout=NULL;
	void* buf;
	uInt size_buf;

	unz_file_info64 file_info;
	uLong ratio=0;
	err = unzGetCurrentFileInfo64(uf,&file_info,filename_inzip,sizeof(filename_inzip),NULL,0,NULL,0);
	if (err!=UNZ_OK)
		throw new exception("error in : zipfile in unzGetCurrentFileInfo");

	size_buf = WRITEBUFFERSIZE;
	buf = (void*)malloc(size_buf);
	if (buf==NULL)
		throw new exception("Error allocating memory");

	p = filename_withoutpath = filename_inzip;
	while ((*p) != '\0')
	{
		if (((*p)=='/') || ((*p)=='\\'))
			filename_withoutpath = p+1;
		p++;
	}

	if ((*filename_withoutpath)=='\0')
	{
		if ((*popt_extract_without_path)==0)
			mymkdir(filename_inzip);
	}
	else
	{
		const char* write_filename;
		int skip=0;

		if ((*popt_extract_without_path)==0)
			write_filename = filename_inzip;
		else
			write_filename = filename_withoutpath;

		err = unzOpenCurrentFilePassword(uf,password);
		if (err!=UNZ_OK)
			throw new exception("error : zipfile in unzOpenCurrentFilePassword");

		if ((skip==0) && (err==UNZ_OK))
		{
			fout=FOPEN_FUNC(write_filename,"wb");
			if ((fout==NULL) && ((*popt_extract_without_path)==0) &&	(filename_withoutpath!=(char*)filename_inzip))
			{
				char c=*(filename_withoutpath-1);
				*(filename_withoutpath-1)='\0';
				makedir(write_filename);
				*(filename_withoutpath-1)=c;
				fout=FOPEN_FUNC(write_filename,"wb");
			}
			if (fout==NULL)
				throw new exception("error : error opening ...");
		}

		if (fout!=NULL)
		{
			do
			{
				err = unzReadCurrentFile(uf,buf,size_buf);
				if (err<0)
					throw new exception("error : zipfile in unzReadCurrentFile");
				if (err>0)
					if (fwrite(buf,err,1,fout)!=1)
						throw new exception("error : writing extracted file");
			}
			while (err>0);
			if (fout)
				fclose(fout);
			if (err==0)
				change_file_date(write_filename,file_info.dosDate,file_info.tmu_date);
		}

		if (err==UNZ_OK)
		{
			err = unzCloseCurrentFile (uf);
			if (err!=UNZ_OK)
				throw new exception("error : zipfile in unzCloseCurrentFile");
		}
		else
			unzCloseCurrentFile(uf); /* don't lose the error */
	}
	free(buf);
	return err;
}

int do_extract(unzFile uf,int opt_extract_without_path,int opt_overwrite,const char* password)
{
	uLong i;
	unz_global_info64 gi;
	int err;
	FILE* fout=NULL;

	err = unzGetGlobalInfo64(uf,&gi);
	if (err!=UNZ_OK)
		printf("error %d with zipfile in unzGetGlobalInfo \n",err);

	for (i=0;i<gi.number_entry;i++)
	{
		if (do_extract_currentfile(uf,&opt_extract_without_path,password) != UNZ_OK)
			break;

		if ((i+1)<gi.number_entry)
		{
			err = unzGoToNextFile(uf);
			if (err!=UNZ_OK)
			{
				printf("error %d with zipfile in unzGoToNextFile\n",err);
				break;
			}
		}
	}

	return 0;
}

bool UnZipFiles(const string &szip,const string &sSaveFolder)
{
	const char *zipfilename=szip.c_str();
	int opt_overwrite=1;
	int opt_extract_without_path=1; // 0 保留文件夹 1 无文件夹
	const char *dirname=sSaveFolder.c_str();

	unzFile uf=NULL;
	zlib_filefunc64_def ffunc;
	fill_win32_filefunc64A(&ffunc);
	uf = unzOpen2_64(zipfilename,&ffunc);

	if (uf!=NULL)
	{
		makedir(dirname);
		_chdir(dirname);
		do_extract(uf,opt_extract_without_path, opt_overwrite, NULL);
		unzClose(uf);
		return true;
	}
	return false;
}

