/*
 *  Author: sumkee911@gmail.com
 *  Date: 2016-12-19
 *  Brief: Zero远控主要服WS务端
 *
 */

#ifndef ZEROSERVER_H
#define ZEROSERVER_H

#include <QObject>
#include "tcpserver.h"
#include "zeroclient.h"
#include <QHash>

class ZeroServer : public QObject
{
    Q_OBJECT
public:
    explicit ZeroServer(QObject *parent = 0);

    // 启动或停止服务端
    void start(int port);
    void stop();

    // 用id来获取ZeroClient
    ZeroClient *client(int id) {
        return mClients[id];
    }

    TcpServer *server() {
        return mServer;
    }

private:
    TcpServer *mServer;         // Tcp服务端
    QHash<int, ZeroClient*> mClients;  // 用ID来索引相应的客户

    // 生成新的id
    int generateId();

signals:
    // 客户登入或登出，主要是告诉窗口控件
    void clientLogin(int id, QString userName,
                     QString ip,int port, QString system);
    void clientLogout(int id);

public slots:
    // 新客户连接
    void newConnection(QTcpSocket *sock);

    // 客户登入
    void login(ZeroClient*, QString userName,
                  QString ip, int port, QString system);

    // 客户登出
    void logout(int id);
};

#endif // ZEROSERVER_H
