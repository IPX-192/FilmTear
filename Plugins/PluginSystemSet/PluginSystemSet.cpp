#include "PluginSystemSet.h"
#include "WidgetSystemSet.h"
#include "WidgetMesLogin.h"
#include "VisAppBus.h"
#include "MesHttpPost.h"
PluginSystemSet::PluginSystemSet()
{
    pluginID = "PluginSystemSet";
    pluginVersion = "1.0.1";
    pluginAuther = "wangwei";
    pluginAuthority = OPERATOR;
    loadOrder = 0;
    showOrder = 4;
    VisAppBus::subscibeEvent(this, "ShowLogin");
    //MES接口总线
    VisAppBus::subscibeEvent(this, "MesValidateNumber");
    VisAppBus::subscibeEvent(this, "MesSaveProcessOpResult");
    VisAppBus::subscibeEvent(this, "MesCompleteTask");
}

void PluginSystemSet::InitWidgetList(Plugin_Interface *plugin)
{
    WidgetSystemSet* widget=new WidgetSystemSet();
    plugin->page=widget;
    plugin->icons << 0xf031 << 0xf036 << 0xf036;
    plugin->iconArea = QSize(40, 40);
    plugin->iconSize = 25;
    plugin->btnHeight = 45;
}

void PluginSystemSet::InitActionList(Plugin_Interface *plugin)
{

}

int PluginSystemSet::OnInitialized()
{
    WidgetSystemSet* widget = (WidgetSystemSet*)this->page;
    widget->LoadUIParam();
    return 0;
}


int PluginSystemSet::event_ShowLogin()
{
    WidgetMesLogin widgetMesLogin;
    QString errMsg;
    widgetMesLogin.LoadAllWorkOrderData(errMsg);
    widgetMesLogin.show();
    widgetMesLogin.raise();          // 确保提到最前
    widgetMesLogin.activateWindow(); // 激活窗口获得焦点

    int nRes = -1;
    QEventLoop* eventloop = new QEventLoop(this);
    connect(&widgetMesLogin, &WidgetMesLogin::sigLogin, [&](QString name, int level) {
        this->frameCore->curUserInfo.userName = name;
        this->frameCore->curUserInfo.authority = (AuthorityType)level;
        eventloop->exit(0);
        nRes = 0;
    });

    connect(&widgetMesLogin, &WidgetMesLogin::sigCloseEvent, [&]() {
        exit(0);
        nRes = -1;
    });
    eventloop->exec();
    return nRes;
}

//==================== MES接口总线 ====================
int PluginSystemSet::event_MesValidateNumber(QString sn, bool& outValidate)
{
    outValidate = false;
    //验证打印:请求实际地址(基址+接口名)
    ShowSystemLog(Log_Info, QString(u8"条码校验请求地址:%1validateNumber").arg(MesHttpPost::Instance()->GetMesBaseUrl()));
    //条码校验前先启动生产任务
    bool startResult = false;
    QString startErr = MesHttpPost::Instance()->StartProduction(sn, 0, startResult);   //boardNum连板数固定1
    if (!startErr.isEmpty() || !startResult) {
        ShowSystemLog(Log_Error, QString(u8"启动生产任务失败:%1").arg(startErr.isEmpty() ? u8"result=false" : startErr));
        return -1;
    }
    ShowSystemLog(Log_Info, QString(u8"启动生产任务成功:%1").arg(sn));
    QString errMsg = MesHttpPost::Instance()->ValidateNumber(sn, outValidate);
    if (!errMsg.isEmpty()) {
        ShowSystemLog(Log_Error, QString(u8"条码校验失败:%1").arg(errMsg));
        return -1;
    }
    ShowSystemLog(Log_Info, QString(u8"条码校验完成:%1,校验结果:%2").arg(sn).arg(outValidate ? u8"通过" : u8"不通过"));
    return 0;
}

int PluginSystemSet::event_MesSaveProcessOpResult(QString sn, int opResult, QList<DataDetail> detailAll, QString& outMainId)
{
    outMainId.clear();
    QString errMsg = MesHttpPost::Instance()->SaveProcessOpResult(sn, opResult, detailAll, outMainId);
    if (!errMsg.isEmpty()) {
        ShowSystemLog(Log_Error, QString(u8"保存工序操作结果失败:%1").arg(errMsg));
        return -1;
    }
    ShowSystemLog(Log_Info, QString(u8"保存工序操作结果完成:%1,mainId:%2").arg(sn).arg(outMainId));
    return 0;
}

int PluginSystemSet::event_MesCompleteTask(QString sn, bool isSuccess, QString errCode, QString errInfo, bool bindMat, bool& outTaskResult)
{
    outTaskResult = false;
    QString errMsg = MesHttpPost::Instance()->CompleteTask(sn, isSuccess, errCode, errInfo, bindMat, outTaskResult);
    if (!errMsg.isEmpty()) {
        ShowSystemLog(Log_Error, QString(u8"工序过站失败:%1").arg(errMsg));
        return -1;
    }
    ShowSystemLog(Log_Info, QString(u8"工序过站完成:%1,结果:%2").arg(sn).arg(outTaskResult ? u8"OK" : u8"NG"));
    return 0;
}

