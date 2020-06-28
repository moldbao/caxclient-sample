#pragma once

#undef ERROR
#include "glog/include/logging.h"

class GLogHelper
{
public:
	static void INIT(const char* program)
	{
		if (_glog == NULL)
			_glog = new GLogHelper(program);
	}

private:
	static GLogHelper *_glog;
	GLogHelper(const char* logName){
		google::InitGoogleLogging(logName);

		google::SetStderrLogging(google::WARNING);					//	设置级别高于 google::INFO的日志同一时候输出到屏幕
		FLAGS_colorlogtostderr=true;								//	设置输出到屏幕的日志显示对应颜色

		google::SetLogDestination(google::INFO,logName);			//	设置 google::INFO 级别的日志存储路径和文件名称前缀		
		google::SetLogDestination(google::WARNING,logName);			//	设置 google::WARNING 级别的日志存储路径和文件名称前缀		
		google::SetLogDestination(google::ERROR,logName);			//	设置 google::ERROR 级别的日志存储路径和文件名称前缀

		FLAGS_logbufsecs =0;										//	缓冲日志输出，默觉得30秒。此处改为马上输出
		FLAGS_max_log_size =1;		 								//	最大日志大小为 100MB 单位 为 MB
		FLAGS_stop_logging_if_full_disk = true;						//	当磁盘被写满时，停止日志输出
		//FLAGS_servitysinglelog = true;							//	用来按照等级区分log文件
		FLAGS_allinonelogfile = true;								//	所有 日志 输出 至 一个 日志 文件中

		FLAGS_max_log_file_count = 10;								//	固定 日志 名称 时 最多 保留的 日志 文件个数
		FLAGS_fixed_log_file_name = true;							//	固定 日志 名称 否则 日志名称 会 增加 日期
	}	
	~GLogHelper(){
		google::ShutdownGoogleLogging();							//	GLOG内存清理：
	}
};

GLogHelper * GLogHelper::_glog = NULL;

#define GINFO	LOG(INFO)
#define GWARN	LOG(WARNING)
#define GERROR	LOG(ERROR)
#define GFATAL	LOG(FATAL)