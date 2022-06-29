#include "tcpserver.h"

TcpServer::TcpServer(QObject *parent) : QObject(parent)
{
    mServer = new QTcpServer(this);
    connect(mServer, SIGNAL(newConnection()), this, SLOT(newConnection()));
}

void TcpServer::start(int port)
{
    if (!mServer->isListening()) {
        if (mServer->listen(QHostAddress::AnyIPv4, port)) {
            qDebug() << "服务端监听成功";
        } else {
            qDebug() << "服务端监听失败：" << mServer->errorString();
        }
    }
}

void TcpServer::stop()
{
    if (mServer->isListening()) {
        mServer->close();
    }
}

void TcpServer::newConnection()
{
    while (mServer->hasPendingConnections()) {
        // 获取新连接
        QTcpSocket *sock = mServer->nextPendingConnection();
        // 发射新连接信号让调用服务器的类知道
        emit newConnection(sock);
    }
}
