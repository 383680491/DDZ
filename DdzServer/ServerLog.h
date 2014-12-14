//
// ServerLog.h
//
//  ��ӡ��������־
//
#pragma once

// �������һ�������־���ַ�����
#define MAX_LOG_LEN                     256

// ��Ч���ַ������������ֵ��ȥ�س����кͽ�������
#define VALID_LOG_LEN                   (MAX_LOG_LEN - 3)

#define LOG_OPT_NUM                     4
// Log level
typedef enum
{
    LOG_INFO    = 1UL,
    LOG_WARN    = 2UL,
    LOG_ERROR   = 4UL,
    LOG_DEBUG   = 8UL
} LOG_LEVEL;


// ��ʼ����������������־����
BOOL StartServerLog(void);
// ֹͣ������־����
void StopServerLog(void);

// set log level
void LogEnableAll(BOOL enable);
void LogEnableInfo(BOOL enable);
void LogEnableWarn(BOOL enable);
void LogEnableError(BOOL enable);
void LogEnableDebug(BOOL enable);

// output log
void WriteLog(LOG_LEVEL level, TCHAR* fmt, ...);
// clear log
void ClearLog(void);
