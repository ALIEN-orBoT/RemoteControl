#include "zeroserver.h"

ZeroServer::ZeroServer(QObject *parent) : QObject(parent)
{
    // 初始化服务器
    mServer = new TcpServer(this);
    connect(mServer, SIGNAL(newConnection(QTcpSocket*)), this, SLOT(newConnection(QTcpSocket*)));
}

void ZeroServer::start(int port)
{
    mServer->start(port);
}

void ZeroServer::stop()
{
    mServer->stop();

    // 清除所有客户
    QHash<int , ZeroClient*> clients = mClients;
    foreach(int id, clients.keys()) {
        clients[id]->closeAndDelete();
    }
    mClients.clear();
}

int ZeroServer::generateId()
{
    const int max = 1 << 30;

    // 避免重复
    QList<int> existsKeys = mClients.keys();
    for (int i=mClients.size()+1; i<max; ++i) {
        if (existsKeys.indexOf(i) == -1) {
            return i;
        }
    }

    return -1;
}

void ZeroServer::newConnection(QTcpSocket *sock)
{
    // 创建ZeroClient，把sock添加进去
    ZeroClient *client = new ZeroClient(sock);
    connect(client, SIGNAL(login(ZeroClient*,QString,QString,int,QString)),
            this, SLOT(login(ZeroClient*,QString,QString,int,QString)));
    connect(client, SIGNAL(logout(int)), this, SLOT(logout(int)));
}

void ZeroServer::login(ZeroClient *client, QString userName, QString ip, int port, QString system)
{
    // 增加客户到哈希表
    int id = generateId();
    mClients.insert(id, client);
    client->setId(id);

    // 发射登入信号给窗口控件
    emit clientLogin(id, userName, ip, port, system);
}

void ZeroServer::logout(int id)
{
    // 从哈希表中删除客户
    mClients.remove(id);

    // 发射登出信号给窗口控件
    emit clientLogout(id);
}

