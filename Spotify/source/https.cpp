#include "https.h"
#include "stringutils.h"
#include <string.h>
#include <net/net.h>
#include <mem.h>
#include <memory.h>
#include <stl/string.h>
#include <printf.h>
#include <VI/vi.h>

namespace HTTPSRequest {
    struct sockaddr_in server;
    void mParseUrl(char *mUrl, char* serverName, char* filepath, char* filename)
    {
        int n;
        char* url = new (Heaps::Network) char[strlen(mUrl) + 1];
        strcpy(url, mUrl);
        url[strlen(mUrl) + 1] = '\0';
        char* temp = new (Heaps::Network) char[9];
        strncpy(temp, url, 7);
        temp[7] = '\0';
        OSReport("%s\n", temp);

        if(strcmp(temp, "http://") == 0)
        {
            StringUtils::removeChars(url, 0, 6);
        }

        strncpy(temp, url, 8);
        temp[8] = '\0';

        if(strcmp(temp, "https://") == 0)
        {
            StringUtils::removeChars(url, 0, 7);
        }
        OSReport("%s\n", url);
        free(temp);
        char *e;
        int index;

        e = strchr(url, '/');
        if(e != NULL)
        {
            index = (int)(e - url);
            
            strncpy(serverName, url, index);
            serverName[index] = '\0';
            
            strncpy(filepath, url + index, strlen(url));
            filepath[strlen(url) + 1] = '\0';

            n = StringUtils::findLastIndex(filepath, '/');
            strncpy(filename, filepath + n + 1, strlen(filepath));
            filename[strlen(filepath) + 1] = '\0';
        }
        else 
        {
            strcpy(serverName, url);
            strcpy(filepath, "/");
            strcpy(filename, "");
        }
    }
    int connectToServer(unsigned long host, char* hostname, s32& sslconnection)
    {
        unsigned int addr;
        int conn;

        conn = socket(AF_INET, SOCK_STREAM, 0);
        if(conn == -1)
        {
            return -1;
        }

        server.sin_addr.s_addr = host;
        server.sin_family = AF_INET;
        server.sin_port = SOHtoNs(443);
        if(connect(conn,(struct sockaddr*)&server,sizeof(server)))
        {
            OSReport("Failed to connect to socket\n");
            closesocket(conn);
            return -1;
        }
        s32 context = SSLNew(0, hostname);
        s32 certResponse = SSLSetBuiltinClientCert(context, 0);
        s32 rootCAResponse = SSLSetBuiltinRootCA(context, 0);
        SSLConnect(context, conn);
        SSLDoHandshake(context);
        sslconnection = context;
        return conn;
    }
    inline char* realloc(char *ptr, int originalLength, int newLength)
    {
        if (newLength == 0)
        {
            free(ptr);
            return NULL;
        }
        else if (!ptr)
        {
            return new(Heaps::Network) char[newLength];
        }
        else if (newLength <= originalLength)
        {
            return ptr;
        }
        else
        {
            char* ptrNew = new(Heaps::Network) char[newLength];
            if (ptrNew)
            {
                memmove(ptrNew, ptr, originalLength);
                free(ptr);
            }
            return ptrNew;
        }
    }
    int getHeaderLength(char *content)
    {
        const char *srchStr1 = "\r\n\r\n", *srchStr2 = "\n\r\n\r";
        char *findPos;
        int ofset = -1;

        findPos = strstr(content, srchStr1);
        if (strstr(content, srchStr1) != NULL)
        {
            ofset = findPos - content;
            ofset += strlen(srchStr1);
        }

        else
        {
            findPos = strstr(content, srchStr2);
            if (findPos != NULL)
            {
                ofset = findPos - content;
                ofset += strlen(srchStr2);
            }
        }
        return ofset;
    }
    char* GET(unsigned long IP, char* szUrl, long &bytesReturnedOut, char **headerOut)
    {
        const int bufSize = 512;
        char readBuffer[bufSize], sendBuffer[bufSize], tmpBuffer[bufSize];
        char *tmpResult, *result;
        int conn;
        char *server, *filepath, *filename;
        server = new(Heaps::Network) char[bufSize];
        filepath = new(Heaps::Network) char[bufSize];
        filename = new(Heaps::Network) char[bufSize];
        long totalBytesRead, thisReadSize, headerLen;

        mParseUrl(szUrl, server, filepath, filename);
        s32 sslContext;
        ///////////// step 1, connect //////////////////////
        conn = connectToServer(IP, server, sslContext);

        ///////////// step 2, send GET request /////////////
        sprintf(tmpBuffer, "GET %s HTTP/1.0", filepath);
        strcpy(sendBuffer, tmpBuffer);
        strcat(sendBuffer, "\r\n");
        OSReport("SERVER: %s\n", server);
        sprintf(tmpBuffer, "Host: %s", server);
        strcat(sendBuffer, tmpBuffer);
        strcat(sendBuffer, "\r\n");
        strcat(sendBuffer, "\r\n");
        SSLWrite(sslContext, sendBuffer, strlen(sendBuffer));

    //    SetWindowText(edit3Hwnd, sendBuffer);
        OSReport("Buffer being sent:\n%s", sendBuffer);

        ///////////// step 3 - get received bytes ////////////////
        // Receive until the peer closes the connection
        totalBytesRead = 0;
        while(1)
        {
            memset(readBuffer, 0, bufSize);
            u32 len = sizeof(server);
            thisReadSize = SSLRead(sslContext, readBuffer, bufSize);
            if ( thisReadSize <= 0 )
            {
                OSReport("READING DONE!\n");
                break;
            }
            OSReport("READ BUFFER: %s\n", readBuffer);
            tmpResult = realloc(tmpResult, strlen(tmpResult), thisReadSize + totalBytesRead);
            memmove(tmpResult+totalBytesRead, readBuffer, thisReadSize);
            totalBytesRead += thisReadSize;
        }
        headerLen = getHeaderLength(tmpResult);
        long contenLen = totalBytesRead-headerLen;
        result = new(Heaps::Network) char[contenLen+1];
        memmove(result, tmpResult+headerLen, contenLen);
        result[contenLen] = 0x0;
        char *myTmp;

        myTmp = new(Heaps::Network) char[headerLen+1];
        strncpy(myTmp, tmpResult, headerLen);
        myTmp[headerLen] = NULL;
        free(tmpResult);
        *headerOut = myTmp;

        bytesReturnedOut = contenLen;
        closesocket(conn);
        return result;
    }
    char* POST(unsigned long IP, char* szUrl, long &bytesReturnedOut, char **headerOut, char **headerIn, int numHeaders)
    {
        const int bufSize = 1028;
        char readBuffer[bufSize], sendBuffer[bufSize], tmpBuffer[bufSize], payload[bufSize];
        char *tmpResult, *result;
        int conn;
        char *server, *filepath, *filename;
        server = new(Heaps::Network) char[bufSize];
        filepath = new(Heaps::Network) char[bufSize];
        filename = new(Heaps::Network) char[bufSize];
        long totalBytesRead, thisReadSize, headerLen;

        mParseUrl(szUrl, server, filepath, filename);
        s32 sslContext;
        ///////////// step 1, connect //////////////////////
        conn = connectToServer(IP, server, sslContext);
        int totalHeadersSize = strlen(headerIn[0]);
        strcat(payload, headerIn[0]);
        for(int i = 1; i < numHeaders; i++)
        {
            strcat(payload, "&");
            strcat(payload, headerIn[i]);
            totalHeadersSize += strlen(headerIn[i]) + 1;
        }
        ///////////// step 2, send GET request /////////////
        sprintf(tmpBuffer, "POST %s HTTP/1.1", filepath);
        strcpy(sendBuffer, tmpBuffer);
        sprintf(tmpBuffer, "\r\nHost: %s\r\nAccept: */*\r\nContent-Type: application/x-www-form-urlencoded\r\nContent-Length: %u\r\nConnection: close\r\n\r\n", server, totalHeadersSize);
        strcat(sendBuffer, tmpBuffer);
        strcat(sendBuffer, payload);
        SSLWrite(sslContext, sendBuffer, strlen(sendBuffer));

    //    SetWindowText(edit3Hwnd, sendBuffer);
        OSReport("Buffer being sent:\n%s", sendBuffer);

        ///////////// step 3 - get received bytes ////////////////
        // Receive until the peer closes the connection
        totalBytesRead = 0;
        while(1)
        {
            memset(readBuffer, 0, bufSize);
            u32 len = sizeof(server);
            thisReadSize = SSLRead(sslContext, readBuffer, bufSize);
            if ( thisReadSize <= 0 )
            {
                OSReport("READING DONE!\n");
                break;
            }
            OSReport("READ BUFFER: %s\n", readBuffer);
            tmpResult = realloc(tmpResult, strlen(tmpResult), thisReadSize + totalBytesRead);
            memmove(tmpResult+totalBytesRead, readBuffer, thisReadSize);
            totalBytesRead += thisReadSize;
        }
        headerLen = getHeaderLength(tmpResult);
        long contenLen = totalBytesRead-headerLen;
        result = new(Heaps::Network) char[contenLen+1];
        memmove(result, tmpResult+headerLen, contenLen);
        result[contenLen] = 0x0;
        char *myTmp;

        myTmp = new(Heaps::Network) char[headerLen+1];
        strncpy(myTmp, tmpResult, headerLen);
        myTmp[headerLen] = NULL;
        free(tmpResult);
        *headerOut = myTmp;

        bytesReturnedOut = contenLen;
        SSLShutdown(sslContext);
        closesocket(conn);
        return result;
    }
}