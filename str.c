#include <string.h>
#include <stdlib.h>

#include "str.h"

char* StrSkipSpace(char* str)
{
	for(; *str == ' ' || *str == '\t'; str++);

	return str;
}

char* StrSkipSpaceEnd(char* str)
{
	size_t len = strlen(str);
	if (len > 0)
	{
		char* p_temp = str + len - 1;
		for(; *p_temp == ' ' || *p_temp == '\t'; p_temp--)
		{
			*p_temp = '\0';
			if (p_temp == str)
				break;
		}
	}

	return str;
}

char* StrSkipWord(char* str)
{
	for(; *str != ' ' && *str != '\t' && *str; str++);

	return str;
}

char* StrRemoveCRLF(char* str)
{
	char* p_temp = str;
	for(; *p_temp != '\r' && *p_temp != '\n' && *p_temp; p_temp++);
	
	*p_temp = '\0';

	return str;
}

unsigned char hexTab[16] = "0123456789ABCDEF";

char* Str2HexStr(const char* str)
{
	size_t strLen = strlen(str);
	size_t i;

	char* hexStr = malloc(strLen * 2 + 1);
	if (NULL == hexStr)
		return NULL;

	for (i = 0; i < strLen; ++i)
	{
		hexStr[i * 2] = hexTab[str[i] >> 4];
		hexStr[i * 2 + 1] = hexTab[str[i] & 0x0F];
	}

	hexStr[strLen * 2] = '\0';

	return hexStr;
}

char* StrFindFirstCharFromSet(char* str, const char* set)
{
	size_t i, j;
	for (i = 0; i < strlen(str); ++i)
	{
		for (j = 0; j < strlen(set); ++j)
			if (str[i] == set[j])
			{
				return str + i;
			}
	}

	return NULL;
}

void StrShrinkSpaces(char* str)
{
	size_t segLen = 0;
	size_t strLen = strlen(str);
	char* pos = StrFindFirstCharFromSet(str, "\t ");
	if (NULL == pos)
	{
		return;
	}

	segLen = strspn(pos, "\t ");
	if (segLen > 1)
	{
		memmove(pos + 1, pos + segLen, strLen + str - pos - segLen + 1);
		StrShrinkSpaces(pos + 1);
	}
}
