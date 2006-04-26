#ifndef __STR_H
#define __STR_H

char* StrSkipSpace(char* str);

char* StrSkipSpaceEnd(char* str);

char* StrSkipWord(char* str);

char* StrRemoveCRLF(char* str);

char* Str2HexStr(const char* str);

char* StrFindFirstCharFromSet(char* str, const char* set);

void StrShrinkSpaces(char* str);

#endif /* __STR_H */

