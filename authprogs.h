#ifndef __AUTH_PROGS_H
#define __AUTH_PROGS_H

const char* CFG_FILENAME = "authprogs.conf";
const char* LOG_FILENAME = "authprogs.log";

#define STR_PATH_MAX 1024
#define STR_BUF_SIZE_MAX 1024

enum ERRORS
{
	ERR_SUCCESS = 240,
	ERR_GENERIC,
	ERR_NO_MEMORY,
	ERR_INVALID_PARAMETER,
	ERR_NOT_VALID,
	ERR_GET_LOG_FILE,
	ERR_GET_CFG_FILE,
	ERR_GET_CLIENT_IP,
	ERR_GET_CLIENT_COMMAND,
	ERR_EXECUTE_COMMAND
};

#endif //__AUTH_PROGS_H
