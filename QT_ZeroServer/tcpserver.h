/*
 *  Author: sumkee911@gmail.com
 *  Date: 2016-12-19
 *  Brief: Zero远控tcp服务端接口
 *
 */


#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QObject>
#include <QTcpServer>

class TcpServer : public QObject
{
    Q_OBJECT
public:
    explicit TcpServer(QObject *parent = 0);

    // 启动服务端
    // @port: 监听的端口
    void start(int port);
    void stop();

    // 反回服务器
    QTcpServer *server() {
        return mServer;
    }

private:
    QTcpServer *mServer;  // 在构造函数里初始化

signals:
    // 当新的连接进来时发送的信号
    // @sock: 新的连接
    void newConnection(QTcpSocket *sock);

public slots:
    // 当有从mServer中接收到新连接后，获取新连接的socket，然后再
    // 发射newConnection信号
    void newConnection();
};

#endif // TCPSERVER_H
