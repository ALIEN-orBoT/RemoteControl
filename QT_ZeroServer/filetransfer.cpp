#include "filetransfer.h"

FileTransfer::FileTransfer(QObject *parent) : QObject(parent)
{
    // 初始化
    mHeader.len = 0;
}

int FileTransfer::startRecvFileServer(QString userName, QString filePath)
{
    // 打开文件
    mFile.setFileName(filePath);
    if (!mFile.open(QFile::WriteOnly)) {
        return -1;
    }

    // 开启新的服务端
    mServer = new TcpServer(this);
    connect(mServer,SIGNAL(newConnection(QTcpSocket*)), this,SLOT(newRecvFileConnection(QTcpSocket*)));

    mServer->start(0);
    if (!mServer->server()->isListening()) {
        qDebug() << "开启文件接收服务端失败";
        deleteLater();
        return -1;
    }

    // 开启进度条窗口
    mProgress = new QProgressDialog();
    mProgress->setWindowTitle(QString("从%0客户下载到%1").arg(userName).arg(filePath));
    mProgress->open(this, SLOT(close()));
    mProgress->setMinimumSize(500,mProgress->height());

    return mServer->server()->serverPort();
}

int FileTransfer::startSendFileServer(QString userName, QString filePath)
{
    // 打开文件
    mFile.setFileName(filePath);
    if (!mFile.open(QFile::ReadOnly)) {
        return -1;
    }

    // 开启新的服务端
    mServer = new TcpServer(this);
    connect(mServer,SIGNAL(newConnection(QTcpSocket*)), this,SLOT(newSendFileConnection(QTcpSocket*)));

    mServer->start(0);
    if (!mServer->server()->isListening()) {
        qDebug() << "开启文件传送服务端失败";
        deleteLater();
        return -1;
    }

     // 开启进度条窗口
    mProgress = new QProgressDialog();
    mProgress->setWindowTitle(QString("上传文件%0到%1客户").arg(filePath).arg(userName));
    mProgress->setMinimumSize(500,mProgress->height());
    mProgress->open(this, SLOT(close()));

    return mServer->server()->serverPort();
}

void FileTransfer::close()
{
    // 释放
    mProgress->deleteLater();
    deleteLater();
    mFile.close();
}

void FileTransfer::newRecvFileConnection(QTcpSocket *socket)
{
    // 新增客户
    mSock = new TcpSocket(socket, this);
    connect(mSock,SIGNAL(newData()), this, SLOT(recvData()));
    connect(mSock, SIGNAL(disconnected()), this, SLOT(close()));

    // 不再监听新客户
    mServer->server()->close();
}

void FileTransfer::newSendFileConnection(QTcpSocket *socket)
{
    // 新增客户
    mSock = new TcpSocket(socket, this);
    connect(mSock, SIGNAL(disconnected()), this, SLOT(close()));

    // 开始传送
    sendData();

    // 不再监听新客户
    mServer->server()->close();
}

void FileTransfer::recvData()
{
    QByteArray *buf = mSock->buffer();
    // 读取头
    if (mHeader.len == 0) {
        if ((unsigned int)buf->size() >= sizeof(FileHeader)) {
            memcpy(&mHeader, buf->data(), sizeof(FileHeader));
            buf->remove(0, sizeof(FileHeader));

            // 当前写入清零
            _curWritten = 0;

            // 设置进度条
            mProgress->setRange(0, mHeader.len);
        }
    }

    if (mHeader.len > 0 && buf->size() > 0) {
        // 增加已写入数据
        _curWritten += buf->size();

        // 写数据到文件里
        mFile.write(buf->data(), buf->size());
        mFile.flush();
        buf->clear();

        // 更新进度条
        mProgress->setValue(_curWritten);
    }

    // 如果已经完成就退出
    if (_curWritten >= mHeader.len) {
        close();
    }
}

void FileTransfer::sendData()
{
    // 发送文件数据
    unsigned int len = mFile.size();
    unsigned int pos = 0;
    unsigned int packetSize = 800;

    // 设置进度条范围
    mProgress->setRange(0, len);

    while (pos < len) {
        int sendSize = pos+packetSize < len ? packetSize : len-pos;
        QByteArray data = mFile.read(sendSize);

        // 发送
        mSock->write(data);

        pos += sendSize;

        // 设置进度条当前位置
        mProgress->setValue(pos);

    }

    // 完成
    close();
}
