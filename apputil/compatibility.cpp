#include "compatibility.h"
#include <cstring>
#include <io.h>


char* my_strtok(char* str, char const * const delim)
{
    return strtok(str, delim);
}

char* my_strcpy(char* dest, char const *src)
{
    return strcpy(dest, src);
}

int my_read(int h, void* dstBuff, unsigned int bufSize)
{
    return read(h, dstBuff, bufSize);
}


int my_open(char const* filename, int openFlag, std::int32_t oper)
{
    return open(filename, openFlag, oper);
}

int my_write(int h, void* dstBuff, unsigned int bufSize)
{
    return read(h, dstBuff, bufSize);
}

int my_close(int h)
{
    return close(h);
}

