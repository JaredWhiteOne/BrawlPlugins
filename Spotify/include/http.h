#pragma once
#include <net/net.h>
#include <memory.h>
#include <OS/OSError.h>
#include <OS/OSThread.h>

namespace HTTPRequest {
    void mParseUrl(char *mUrl, char* serverName, char* filepath, char* filename);
    int connectToServer(unsigned long host, short portNum);
    int getHeaderLength(char *content);
    char* GET(unsigned long IP, char* szUrl, long &bytesReturnedOut, char **headerOut);
    char* POST(unsigned long IP, char* szUrl, long &bytesReturnedOut, char **headerOut, char **headerIn, int numHeaders);
}