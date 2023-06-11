#pragma once

#include <net/net.h>
#include <memory.h>
#include <OS/OSError.h>
#include <OS/OSThread.h>
#include <net/ssl.h>

namespace HTTPSRequest {
    void mParseUrl(char *mUrl, char* serverName, char* filepath, char* filename);
    int connectToServer(unsigned long host, char* hostname, s32& sslconnection);
    int getHeaderLength(char *content);
    char* GET(unsigned long IP, char* szUrl, long &bytesReturnedOut, char **headerOut);
    char* POST(unsigned long IP, char* szUrl, long &bytesReturnedOut, char **headerOut, char **headerIn, int numHeaders);
}