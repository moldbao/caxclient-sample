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

		google::SetStderrLogging(google::WARNING);					//	���ü������ google::INFO����־ͬһʱ���������Ļ
		FLAGS_colorlogtostderr=true;								//	�����������Ļ����־��ʾ��Ӧ��ɫ

		google::SetLogDestination(google::INFO,logName);			//	���� google::INFO �������־�洢·�����ļ�����ǰ׺		
		google::SetLogDestination(google::WARNING,logName);			//	���� google::WARNING �������־�洢·�����ļ�����ǰ׺		
		google::SetLogDestination(google::ERROR,logName);			//	���� google::ERROR �������־�洢·�����ļ�����ǰ׺

		FLAGS_logbufsecs =0;										//	������־�����Ĭ����30�롣�˴���Ϊ�������
		FLAGS_max_log_size =1;		 								//	�����־��СΪ 100MB ��λ Ϊ MB
		FLAGS_stop_logging_if_full_disk = true;						//	�����̱�д��ʱ��ֹͣ��־���
		//FLAGS_servitysinglelog = true;							//	�������յȼ�����log�ļ�
		FLAGS_allinonelogfile = true;								//	���� ��־ ��� �� һ�� ��־ �ļ���

		FLAGS_max_log_file_count = 10;								//	�̶� ��־ ���� ʱ ��� ������ ��־ �ļ�����
		FLAGS_fixed_log_file_name = true;							//	�̶� ��־ ���� ���� ��־���� �� ���� ����
	}	
	~GLogHelper(){
		google::ShutdownGoogleLogging();							//	GLOG�ڴ�����
	}
};

GLogHelper * GLogHelper::_glog = NULL;

#define GINFO	LOG(INFO)
#define GWARN	LOG(WARNING)
#define GERROR	LOG(ERROR)
#define GFATAL	LOG(FATAL)