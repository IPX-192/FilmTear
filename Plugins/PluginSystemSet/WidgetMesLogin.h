#ifndef WIDGETMESLOGIN_H
#define WIDGETMESLOGIN_H

#include <QWidget>
#include <QMessageBox>
#include <QVector>
#include <QTimer>
#include "MesHttpPost.h"
#include "ControlCenterHttpApi.h"
#include "WidgetLoad.h"

namespace Ui {
class WidgetMesLogin;
}

const int HEARTBEAT_INTERVAL = 5000;

class WidgetMesLogin : public QWidget
{
    Q_OBJECT

public:
    explicit WidgetMesLogin(QWidget *parent = nullptr);
    ~WidgetMesLogin();

protected:
    void closeEvent(QCloseEvent *event) override;

public:
    bool LoadAllWorkOrderData(QString& errMsg);

signals:
    //通知其他类进行治具检测和自检
    void SignalCheckFixtureNeedAndSelfCheck();

    void sigLogin(QString username, int level);
    void sigCloseEvent();

private slots:
    void on_comboBox_pdLineList_currentIndexChanged(int index);
    void on_comboBox_pdList_currentIndexChanged(int index);
    void on_comboBox_workOrderList_currentIndexChanged(int index);

    void on_pushButton_login_clicked();

private slots:
    void SlotPostHeartBeatTimeout();

    void on_btn_UpdateList_clicked();

    void onLoadDone();

private:
    // 工具函数
    void ShowMsg(const QString& title, const QString& text);
    void SetUiLock(bool lock);

    bool RefreshProcessCombo(const QString& workOrderId, QString& errMsg);
    bool DoFullLoginFlow(QString& errMsg);

private:
    Ui::WidgetMesLogin *ui;
    QMessageBox* m_waitMessageBox;
    WidgetLoad *m_widgetLoad = nullptr;
    QVector<PdLineList> m_allPdLineData;
    QList<MaterialInfo> m_materialInfo;
    bool g_success = false;
    bool m_load = false;
    QTimer m_heartTimer;         //发送心跳定时器
};

#endif // WIDGETMESLOGIN_H
