#ifndef MODELCHANGEMANAGER_H
#define MODELCHANGEMANAGER_H

#include <QObject>
#include <QTimer>
#include <QString>
#include <QDateTime>
#include <QJsonArray>
#include <QList>
#include "ControlCenterHttpApi.h"
#include "MesHttpPost.h"

// 换型最终结果枚举
enum ModelChangeResult
{
    Result_Success,
    Result_Fail_NoTask,         // 获取任务失败
    Result_Fail_CannotChange,   // 设备不可换型
    Result_Fail_MissingFile,    // 配方切换NG缺少文件
    Result_FixtureCheck_Fail,   // 治具校验失败
    Result_SelfCheck_Fail       // 自检NG
};

// 流程状态枚举
enum ModelStatus
{
    Status_None,        // 空闲无流程
    Status_Prepare,     // 收到ModelChangePrepare，等待Complete指令
    Status_Complete     // 收到ModelChangeComplete，执行业务流程
};

// 常量定义
const int POLL_INTERVAL = 10000;        // 10s轮询指令

class ModelChangeManager : public QObject
{
    Q_OBJECT
public:
    explicit ModelChangeManager(QObject *parent = nullptr);
    ~ModelChangeManager();

    void StartModelChange(const QString& deviceCode, const QString& deviceIp);
    void StopModelChange();
    QString GetLastErrorMsg() const;

signals:
    void SignalModelChangeFinished(ModelChangeResult result, const QString& msg);
    void SignalAlarmTrigger(const QString& alarmInfo);

private slots:
    void SlotPollInstruction();

private:
    void ProcessInstruction();
    // 收到ModelChangeComplete指令后进入任务获取流程
    void NotifyLocalModelComplete();
    // 统一流程收尾，重置状态并抛出结束信号
    void ReportModelResult(ModelChangeResult res, const QString& extraMsg);

    // 1. 处理配方切换
    bool HandleProdSwitch(const QString& changeRes, QString& outErr);
    // 2. 处理治具校验
    bool HandleFixtureCheck(QList<BindFixtureItem>& fixtureList, QString& outErr);
    // 3. 处理自检结果
    bool HandleSelfCheck(const QString& selfRes, QString& outErr);

    // 拉取控制中心下发指令
    bool PullDeviceInstruction(DeviceExecCommand& outCmd, QString& errMsg);
    // 获取当前生产任务
    bool PullCurrentTask(DeviceTaskInfo& outTask, QString& errMsg);
    // MES治具校验逻辑
    bool CheckFixtureNeed(QList<BindFixtureItem>& list, QString& errMsg);

    //上报接口
    // 上报指令执行结果
    void UploadCmdResult(const QString& cmd, bool isSuccess, const QString& msg = "");
    // 上报告警事件
    void UploadAlarm(const QString& alarmText);
    // 上报设备运行状态
    void UploadDeviceStatus(const QString& status);

private:
    ControlCenterHttpApi* m_ctrlApi;
    MesHttpPost* m_mesApi;
    QTimer m_pollTimer;
    QString m_deviceCode;
    QString m_deviceIp;
    QString m_lastError;
    bool m_waitCompleteFlag;
    ModelStatus m_modelStatus;
};

#endif // MODELCHANGEMANAGER_H
