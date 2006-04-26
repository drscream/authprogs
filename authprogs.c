/*
 * Copyright (c) 2006 Cristian Sandu <csandu4096@yahoo.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <sys/wait.h>
#include <unistd.h>
#include <pwd.h>
#include <errno.h>
#include <stdlib.h>
#include <limits.h>

#include "safe_string.h"
#include "str.h"
#include "log.h"
#include "version.h"
#include "authprogs.h"

static int GetDefaultPath(char* path, size_t len, const char* filename)
{
	const char* homeVar; 
	
	assert(filename != NULL);
	
	homeVar = getenv("HOME"); 
	
	if (NULL == homeVar)
	{
		Log(ERROR_LEVEL, "Can not get HOME environment variable\n");
		return ERR_GENERIC;
	}

	if (*homeVar == '\0')
	{
		Log(ERROR_LEVEL, "Empty HOME environment variable\n");
		return ERR_GENERIC;
	}

	strlcpy(path, homeVar, len);
	strlcat(path, "/.ssh/", len);
	strlcat(path, filename, len);
	
	return 0;
}

static FILE* GetLogFile(char* path)
{
	//get path to log file
	FILE* logFile;
	char logPath[STR_PATH_MAX];
	int rc ;

	if (NULL == path)
	{
		rc = GetDefaultPath(logPath, STR_PATH_MAX, LOG_FILENAME);
		if (rc != 0)
		{
			Log(ERROR_LEVEL, "Can not get default log file path (%s)\n", rc);
			return NULL;
		}
		path = logPath;
	}

	//open log file
	logFile = fopen(path, "at");
	if (NULL == logFile)
		Log(ERROR_LEVEL, "Can not open log file (%s)\n", path);

	return logFile;
}

static FILE* GetCfgFile(char* path)
{
	FILE* cfgFile;
	char cfgPath[STR_PATH_MAX];
	int rc;
	
	if (NULL == path)
	{
		//get path to configuration file
		rc = GetDefaultPath(cfgPath, STR_PATH_MAX, CFG_FILENAME);
		if (rc != 0)
		{
			Log(ERROR_LEVEL, "Can not get default configuration file path (%s)\n", rc);
			return NULL;
		}
		path = cfgPath;
	}
	
	//open configuration file
	cfgFile = fopen(path, "rt");
	if (NULL == cfgFile)
		Log(ERROR_LEVEL, "Can not open configuration file (%s)\n", path);

	return cfgFile;
}

static int GetClientIp(char* clientIp, size_t len)
{
	//get SSH_CLIENT variable
	char* pos;
	const char* sshClientVar = getenv("SSH_CLIENT"); 
	if (NULL == sshClientVar)
	{
		Log(ERROR_LEVEL, "Can not get SSH_CLIENT environment variable\n");
		return ERR_GENERIC;
	}

	if (*sshClientVar == '\0')
	{
		Log(ERROR_LEVEL, "Empty SSH_CLIENT environment variable\n");
		return ERR_GENERIC;
	}

	pos = StrSkipSpace((char*)sshClientVar);
	if (*pos == '\0')
	{
		Log(ERROR_LEVEL, "Empty SSH_CLIENT environment variable\n");
		return ERR_GENERIC;
	}

	strlcpy(clientIp, pos, len);

	pos = StrSkipWord(clientIp);
	*pos = '\0';
	
	return 0;
}

static int GetClientCommand(char clientCommand[], size_t len)
{
	char* pos;

	//get SSH_ORIGINAL_COMMAND variable
	const char* sshCommandVar = getenv("SSH_ORIGINAL_COMMAND"); 
	if (NULL == sshCommandVar)
	{
		Log(ERROR_LEVEL, "Can not get SSH_ORIGINAL_COMMAND environment variable\n");
		return ERR_GENERIC;
	}

	if (*sshCommandVar == '\0')
	{
		Log(ERROR_LEVEL, "Empty SSH_ORIGINAL_COMMAND environment variable\n");
		return ERR_GENERIC;
	}

	if (g_currentLogLevel == DEBUG_LEVEL)
	{
		char* hexStr = Str2HexStr(sshCommandVar);
		if (hexStr == NULL)
		{
			Log(ERROR_LEVEL, "Can not allocate memory (hexStr)\n");
			return ERR_NO_MEMORY;
		}

		Log(DEBUG_LEVEL, "Client command in hexadecimal (%s)\n", hexStr);
		free(hexStr);
	}

	pos = StrSkipSpace((char*)sshCommandVar);
	if (*pos == '\0')
	{
		Log(ERROR_LEVEL, "Empty SSH_ORIGINAL_COMMAND environment variable\n");
		return ERR_GENERIC;
	}

	strlcpy(clientCommand, pos, len);

	clientCommand = StrSkipSpaceEnd(clientCommand);
	
	StrShrinkSpaces(clientCommand);

	return 0;
}

static int ValidateIp(char* line, const char* clientIp)
{
	char* token;
	size_t clientIpLen = strlen(clientIp);

	/*Log(DEBUG_LEVEL, "line, clientIp (%s, %s)\n", line, clientIp);*/

	token = StrSkipSpace(line);

	if (0 == strncasecmp(token, "ALL", 3))
		return 0;

	StrShrinkSpaces(token);

	/*Log(DEBUG_LEVEL, "token (%s)\n", token);*/
	
	while (*token != '\0')
	{
		if (!strncmp(token, clientIp, clientIpLen))
			return 0;

		token = StrSkipSpace(StrSkipWord(token));
		/*Log(DEBUG_LEVEL, "token (%s)\n", token);	*/
	}
	
	return ERR_NOT_VALID;
}

static int Validate(FILE* cfgFile, char* clientIp, char* clientCommand)
{
	char buf[STR_BUF_SIZE_MAX];
	unsigned int ipMatch = 0;
	
	if ((NULL == cfgFile)	||
		(NULL == clientIp)	||
		(NULL == clientCommand))
	{
		return ERR_INVALID_PARAMETER;
	}

	while (fgets(buf, STR_BUF_SIZE_MAX, cfgFile) != NULL)
	{
		size_t strLen;
		
		char* p_str = StrSkipSpaceEnd(StrRemoveCRLF(StrSkipSpace(buf)));

		/*Log(DEBUG_LEVEL, "(%s, %s, %s)\n", p_str, clientIp, clientCommand);*/
		
		strLen = strlen(p_str);
		//skip comments and empty lines
		if (('#' == p_str[0])	||
			(';' == p_str[0])	||
			(0 == strLen)		||
			(('[' == p_str[0]) && (p_str[strLen - 1] != ']')) ||
			((p_str[0] != '[') && (']' == p_str[strLen - 1])))
		{
			continue;
		}

		if (('[' == p_str[0]) && (']' == p_str[strLen - 1]))
		{
			p_str[strLen - 1] = '\0';
			p_str++;

			ipMatch = 0;
			if (ValidateIp(p_str, clientIp) == 0)
				ipMatch = 1;

			continue;
		}
		
		if (ipMatch)
		{
			StrShrinkSpaces(p_str);
			if (0 == strcmp(p_str, clientCommand))
				return 0;
		}
	}
	
	return ERR_NOT_VALID;
}

/*void PrintEnv()
{
	extern char** environ;
	if (NULL == environ)
	{
		Log(DEBUG_LEVEL, "Can not get environment\n");
		return;
	}

	char** senviron = environ;
	while (*senviron != NULL)
	{
		Log(DEBUG_LEVEL, "%s\n", *senviron++);
	}
}*/

static void Usage()
{
	fprintf(stdout, "\nauthprogs version %u.%u.%u\n"
					"\nUsage :\n"
					"\tauthprogs [-c config] [-l log_level]\n"
					"\t\t-c config\tconfiguration file\n"
					"\t\t-l log_level\tlog level (0 - quiet,1 - error,2 - fail cmd,3 - success cmd,4 - debug)\n"
					"\t\t-h help\n\n",
					AUTHPROGS_MAJOR, AUTHPROGS_MINOR, AUTHPROGS_EXTERNAL_BUILD);
}

int main(int argc, char* argv[])
{
	int rc = 0;
	
	FILE* cfgFile = NULL;
	char* cfgPath = NULL;
	
	char clientIp[STR_BUF_SIZE_MAX];
	char clientCommand[STR_BUF_SIZE_MAX];
	
	FILE* logFileTmp = NULL;
	
	gp_logFile = stdout;
	g_currentLogLevel = SCMD_LEVEL;

	switch (argc)
	{
		case 0 :
			Log(ERROR_LEVEL, "argv[0] is missing\n");
			return ERR_GENERIC;
			
		case 1 :
			break;

		default :
			if (argc > 5)
			{
				Log(ERROR_LEVEL, "Too many arguments\n");
				Usage();
				return ERR_GENERIC;
			}
			
			++argv;
			while (*argv != NULL)
			{
				if ((0 == strcmp(*argv, "-c")) && (*(argv + 1) != NULL))
				{
					cfgPath = *(argv + 1);
					argv += 2;
				}
				else if ((0 == strcmp(*argv, "-l")) && (*(argv + 1) != NULL))
				{
					g_currentLogLevel = strtoul(*(argv + 1), NULL, 10);
					if ((ULONG_MAX == g_currentLogLevel) ||
						(g_currentLogLevel >= DEBUG_LEVEL))
					{
						g_currentLogLevel = DEBUG_LEVEL;
					}
					argv += 2;
				}
				else if (0 == strcmp(*argv, "-h"))
				{
					Usage();
					return 0;
				}
				else
				{
					Log(ERROR_LEVEL, "Invalid argument (%s)\n", *argv);
					Usage();
					return ERR_GENERIC;
				}
			}
	}
	
	//get log file
	logFileTmp = GetLogFile(NULL);
	if (NULL == logFileTmp)
	{
		rc = ERR_GET_LOG_FILE;
		goto cleanup;
	}
	
	gp_logFile = logFileTmp;

	//get configuration file
	cfgFile = GetCfgFile(cfgPath);
	if (NULL == cfgFile)
	{
		rc = ERR_GET_CFG_FILE;
		goto cleanup;
	}
	
	//get client ip
	rc =  GetClientIp(clientIp, STR_BUF_SIZE_MAX);
	if (rc != 0)
	{
		rc = ERR_GET_CLIENT_IP;
		goto cleanup;
	}

	//get client command
	rc = GetClientCommand(clientCommand, STR_BUF_SIZE_MAX);
	if (rc != 0)
	{
		rc = ERR_GET_CLIENT_COMMAND;
		goto cleanup;
	}

	rc = Validate(cfgFile, clientIp, clientCommand);
	if (rc != 0)
	{
		Log(FCMD_LEVEL, "IP (%s) not allowed to execute command (%s)\n", clientIp, clientCommand);
		fprintf(stderr, "Your IP (%s) is not allowed to execute command (%s)\n", clientIp, clientCommand);
		goto cleanup;
	}
	
	fflush(gp_logFile);
	
	rc = system(clientCommand);
	if (-1 == rc)
	{
		Log(ERROR_LEVEL, "Can not execute command (%s)\n", clientCommand);
		rc = ERR_EXECUTE_COMMAND;
		goto cleanup;
	}
	
	rc = WEXITSTATUS(rc);	
	Log(SCMD_LEVEL, "Successfully executed command (%s) from IP (%s)\n", clientCommand, clientIp);

cleanup :
	//PrintEnv();
	
	if (gp_logFile != stdout)
	{
		fclose(gp_logFile);
	}
	if (cfgFile != NULL)
	{
		fclose(cfgFile);
	}
	
	return rc;
}


