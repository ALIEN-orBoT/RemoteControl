/*
 *  Author: sumkee911@gmail.com
 *  Date: 2016-12-21
 *  Brief: 从客户端接收屏幕图片数据，然后画在窗口上
 *
 */

#ifndef SCREENSPY_H
#define SCREENSPY_H

#include <QWidget>
#include <tcpsocket.h>
#include <tcpserver.h>
#include <QApplication>
#include <QDesktopWidget>
#include <QPixmap>
#include <QLabel>

class ScreenSpy : public QWidget
{
    Q_OBJECT
public:
    explicit ScreenSpy( QWidget *parent = 0);

    // 数据头
    typedef struct {
        unsigned int len;    // jpg文件大小
    } ScreenSpyHeader;


    // 开始监控服务器，然后返回新的端口号
    int startScreenSpyServer(QString userName);

private:
    QLabel *mScreenLabel;        // 用来显示图片
    TcpServer *mServer;   // 屏幕监控服务端
    TcpSocket *mSock;   // 屏幕监控客户端
    QPixmap mScreenPixmap;    // 当前屏幕截图
    unsigned int mScreenLen;    // JPG图片大小

signals:

public slots:
    // 有新客户连接
    void newConnection(QTcpSocket *s);

    // 处理新数据
    void processBuffer();

protected:
    // 大小重置
    void resizeEvent(QResizeEvent *);

    // 关闭
    void closeEvent(QCloseEvent *);
};

#endif // SCREENSPY_H
