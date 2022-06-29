/*
 *
 *  Author: sumkee911@gmail.com
 *  Date: 20-12-2016
 *  Brief: Socket通讯接口
 *
 */

#ifndef TCPSOCKET_H
#define TCPSOCKET_H

#include <winsock2.h>
#include <iostream>
#include <string>
#include <cstdio>

class TcpSocket
{
public:
    TcpSocket();

    // 域名转IP
    static std::string fromDomainToIP(std::string domain);

    // 连接，断开，发送，接收
    bool connectTo(std::string domain, int port);
    void dissconnect();
    bool sendData(const char *data, unsigned int size);
    int recvData(char *data, int size);

    // 判断是否处于连接的状态
    bool isConnected() {
        return (int)mSock != SOCKET_ERROR;
    }

    std::string mIp;         // ip
    int mPort;              // 端口

private:
    SOCKET mSock;           // socket
    struct sockaddr_in mAddr;   // 服务器地址
};

#endif // TCPSOCKET_H
