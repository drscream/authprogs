#include <time.h>
#include <stdarg.h>
#include <string.h>

#include "safe_string.h"
#include "log.h"

FILE* gp_logFile;
LogLevel g_currentLogLevel;

static char g_logBuf[LOG_BUF_LEN_MAX];

static const char* Level2Str(unsigned int level)
{
	switch (level)
	{
		case QUIET_LEVEL :
			return "quiet";

		case ERROR_LEVEL :
			return "error";

		case FCMD_LEVEL :
			return "fail";

		case SCMD_LEVEL :
			return "success";

		case DEBUG_LEVEL :
			return "debug";
	}

	return "unknown";
}

void Log(LogLevel level, const char* format, ...)
{
	size_t logBufSize = sizeof(g_logBuf);
	va_list args;
	size_t logBufLen = 0;
	time_t t;

	if (level > g_currentLogLevel)
		return;

	t = time(NULL);
	if (t != (time_t)-1)
	{
		struct tm* p_tm = localtime(&t);
		if (p_tm != NULL)
			if (strftime(g_logBuf, logBufSize, LOG_TIME_FORMAT, p_tm) == 0)
				g_logBuf[0] = '\0';
	}

	strlcat(g_logBuf, " [", logBufSize);
	strlcat(g_logBuf, Level2Str(level), logBufSize);
	strlcat(g_logBuf, "] ", logBufSize);

	va_start(args, format);

	logBufLen = strlen(g_logBuf);
	vsnprintf(g_logBuf + logBufLen, logBufSize - logBufLen, format, args);

	va_end(args);

	fprintf(gp_logFile, "%s", g_logBuf);
}
