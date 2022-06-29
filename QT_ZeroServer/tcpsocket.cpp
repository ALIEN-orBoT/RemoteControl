#include "tcpsocket.h"

TcpSocket::TcpSocket(QTcpSocket *sock, QObject *parent):
    QObject(parent), mSock(sock)
{
    mSock->setParent(this);
    connect(mSock, SIGNAL(readyRead()), this, SLOT(readReady()));
    connect(mSock, SIGNAL(disconnected()), this, SIGNAL(disconnected()));
    connect(mSock, SIGNAL(disconnected()), this, SLOT(deleteLater()));

    // 输出信息
    qDebug() << mSock->peerAddress().toString() << ":" << mSock->peerPort() << " 已连接上服务端";
}

void TcpSocket::close()
{
    mSock->disconnectFromHost();
    mSock->close();
}

void TcpSocket::write(QByteArray data)
{
    mSock->write(data);

    if (!mSock->waitForBytesWritten(3000)) {
        // 发送数据超时
        close();
        emit disconnected();

        // 输出信息
        qDebug() << mSock->peerAddress().toString() << ":" << mSock->peerPort() \
                 << " 写入失败：" << mSock->errorString();
    }
}

void TcpSocket::readReady()
{
    mBuf.append(mSock->readAll());
    emit newData();
}
