#include "tcpsocket.h"

TcpSocket::TcpSocket() : mSock(SOCKET_ERROR)
{

}

std::string TcpSocket::fromDomainToIP(std::string domain)
{
    // 获取主机信息
    struct hostent *ht = gethostbyname(domain.data());
    if(!ht) {
        std::cout << "Failed to get host" << std::endl;
        std::fflush(stdout);
        return std::string();
    }

    // 分解IPV4地址
    std::string ip;
    for(int i = 0;i < ht->h_length;i ++) {
        char szTmp[10];
        std::sprintf(szTmp, "%d.",(unsigned char)ht->h_addr_list[0][i]);
        ip.append(szTmp);
    }
    ip.pop_back();
    return ip;
}

bool TcpSocket::connectTo(std::string domain, int port)
{
    if ((int)mSock != SOCKET_ERROR) {
        std::cout <<  "Socket is using" << std::endl;\
        std::fflush(stdout);;
        return false;
    }

    mPort = port;

    // 把域名转换成ip
    mIp = fromDomainToIP(domain);

    // 创建socket
    if ((int)(mSock = socket(AF_INET,SOCK_STREAM,0)) == SOCKET_ERROR) {
        std::cout << "Failed to create socket " << std::endl;
        std::fflush(stdout);
        return false;
    }
    mAddr.sin_family = AF_INET;
    mAddr.sin_addr.S_un.S_addr = inet_addr(mIp.data());
    mAddr.sin_port = htons(mPort);

    // 连接至服务端
    if (connect(mSock, (SOCKADDR *)&mAddr, sizeof(mAddr)) == SOCKET_ERROR) {
        std::cout << "Failed to connect to " << mIp << std::endl;
        std::fflush(stdout);
        dissconnect();
        return false;
    }

    std::cout << "Connect to success " << mIp << std::endl;
    std::fflush(stdout);;

    return true;
}

void TcpSocket::dissconnect()
{
    if ((int)mSock != SOCKET_ERROR) {
        closesocket(mSock);
        mSock = SOCKET_ERROR;

        std::cout << "Closed socket from " << mIp << std::endl;
        std::fflush(stdout);;
    }
}

bool TcpSocket::sendData(const char *data,unsigned int size)
{
    if ((int)mSock == SOCKET_ERROR) {
        std::cout << "Socket do not allowed to send data without connected " << std::endl;
        std::fflush(stdout);;
        return false;
    }

    int ret = SOCKET_ERROR;
    const unsigned int packetLen = 800;
    // 小于pakcetLen的话，就直接发送，不然就分包发送
    if (size <= packetLen) {
        ret = send(mSock, data, size, 0);

        // 出现错误，断开连接
        if (ret == SOCKET_ERROR) {
            std::cout <<  "Failed to send data to " <<  mIp << std::endl;
            std::fflush(stdout);;
            dissconnect();
        }
    } else {
        unsigned int pos = 0;
        while (pos < size) {
            unsigned int sendSize = pos+packetLen > size ? size-pos : packetLen;

            // 发送
            ret = send(mSock, data+pos, sendSize, 0);

            // 出现错误，断开连接
            if (ret == SOCKET_ERROR) {
                std::cout <<  "Failed to send data to " <<  mIp << std::endl;
                std::fflush(stdout);;
                dissconnect();
                break;
            }

            pos += packetLen;
        }
    }

    return ret != SOCKET_ERROR ? true : false;
}

int TcpSocket::recvData(char *data, int size)
{
    if ((int)mSock == SOCKET_ERROR) {
        std::cout << "Socket do not allowed to reveive data without connected " << std::endl;
        std::fflush(stdout);;
        return -1;
    }

    int ret = recv(mSock, data, size, 0);

    // 出现错误，断开连接
    if (ret == SOCKET_ERROR || ret == 0) {
        std::cout <<  "Failed to receive data from " << mIp << std::endl;
        std::fflush(stdout);;
        dissconnect();
    }

    return ret;
}

