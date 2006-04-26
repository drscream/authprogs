#ifndef __LOG_H
#define __LOG_H

#include <stdio.h>

typedef enum LogLevel
{
	QUIET_LEVEL = 0,
	ERROR_LEVEL,
	FCMD_LEVEL,
	SCMD_LEVEL,
	DEBUG_LEVEL
} LogLevel;

#define LOG_BUF_LEN_MAX 1024
#define LOG_TIME_FORMAT	"%Y/%m/%d %H:%M:%S"

extern FILE* gp_logFile;
extern LogLevel g_currentLogLevel;

void Log(LogLevel level, const char* p_format, ...);

#endif /* __LOG_H */

