#pragma once

void ErrorExit(char* lpszFunction, const char* file, int line);

#define CHECK_WIN_ERROR \
    do { \
        ErrorExit("", __FILE__, __LINE__); \
    } while(false)