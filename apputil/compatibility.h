#pragma once

#include <cstdint>

char* my_strtok(char* str, char const * const delim);

char* my_strcpy(char* dest, char const *src);

int my_read(int h, void* dstBuff, unsigned int bufSize);

int my_open(char const* filename, int openFlag, std::int32_t oper = 0);

int my_write(int h, void* dstBuff, unsigned int bufSize);

int my_close(int h);

