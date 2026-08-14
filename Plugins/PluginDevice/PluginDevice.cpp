#include "PluginDevice.h"
#include "ParamManager.h"
#include "VisAppBus.h"
#include "VisCameraTool.h"
#include "LightSourceManager.h"

#include "VisAppThreadPool.h"
#include <QCoreApplication>
#include <QMessageBox>
#include <QtCore/QMetaType>
#include "TrayRfidManager.h"
Q_DECLARE_METATYPE(QVariant)

PluginDevice::PluginDevice()
{
    pluginID = "PluginDevice";
    pluginVersion = "1.0.1";
    pluginAuther = "wangwei";
    loadOrder = 2;

    VisAppBus::subscibeEvent(this, "ScanCodeConnect");
    VisAppBus::subscibeEvent(this, "DisScanCodeConnect");

    VisAppBus::subscibeEvent(this, "PressureSensorConnect");
    VisAppBus::subscibeEvent(this, "PressureSensorDisconnect");
    VisAppBus::subscibeEvent(this, "PressureSensorGetRealPressure");
    VisAppBus::subscibeEvent(this, "PressureSensorGetPeakPressure");
    VisAppBus::subscibeEvent(this, "PressureSensorResetPeak");

    VisAppBus::subscibeEvent(this, "DisplacementSensorConnect");
    VisAppBus::subscibeEvent(this, "DisplacementSensorDisconnect");
    VisAppBus::subscibeEvent(this, "DisplacementSensorReadHeight");
    VisAppBus::subscibeEvent(this, "DisplacementSensorResetZero");

    VisAppBus::subscibeEvent(this,"ConnectModbus");
    VisAppBus::subscibeEvent(this,"DisconnectedModbus");
    VisAppBus::subscibeEvent(this,"GetModbusTrayCode");
    VisAppBus::subscibeEvent(this,"GetModbusRFIDExist");
    VisAppBus::subscibeEvent(this,"SetModbusModel");
    VisAppBus::subscibeEvent(this,"SetModbusTrayCode");
    VisAppBus::subscibeEvent(this,"TriggerReadCard");
    VisAppBus::subscibeEvent(this,"ReadRfidTag");
    qRegisterMetaType<QVariant>();
}

PluginDevice::~PluginDevice()
{
    InitOtherCam(false);
    m_scanCodeTray.Disconnected();
    // 断开传感器
    m_pressureClient.Disconnected();
    m_displacementClient.DisconnectPort();
    TrayRfidManager::instance()->disConnect();
}

int PluginDevice::OnViewClosing()
{
    //主窗口关闭时立即关闭相机释放驱动资源,避免快速重启软件时驱动未释放导致打开失败
    InitOtherCam(false);
    m_connectFlag = false;
    m_scanCodeTray.Disconnected();
    m_pressureClient.Disconnected();
    m_displacementClient.DisconnectPort();
    TrayRfidManager::instance()->disConnect();
    ShowSystemLog(Log_Info, QString(u8"相机资源已释放"));
    return 0;
}

void PluginDevice::InitActionList(Plugin_Interface *plugin)
{
    PluginActionInfo* action = new PluginActionInfo();
    action->_actionName = "ReConnetCam";
    action->_actionDetail = tr("重连相机");
    action->_pAction = (FPTR_ACTION)(&PluginDevice::ReConnetCam);
    plugin->listAction.append(action);
}

int PluginDevice::OnInitialized()
{
    // 光源串口管理器初始化
    LightSourceManager::instance();

    // 相机配置初始化
    QString path = QCoreApplication::applicationDirPath() + "/Config/Camera.xml";
    if (!VisCameraTool::instance()->LoadConfig(path))
        ShowSystemLog(Log_Error, QString(u8"相机配置文件初始化失败!"));

    else
    {
        // 后台线程初始化相机
        GlobalThreadPool->Commit(std::bind(&PluginDevice::InitDevice, this));
        // 设置客户端工位名称，总线过滤用
        m_scanCodeTray.SetName(ScanHolderCode);
        m_scanCodePCB.SetName(ScanPCBCode);
        m_pressureClient.SetName(m_stationName);
        m_displacementClient.SetName(m_stationName);
        // 主线程初始化传感器(Modbus/QSerialPort需主线程事件循环)
        int nRet = InitPressureSensor();
        if (nRet == 0) nRet = InitDisplacementSensor();
        if (nRet == 0) {
            ShowSystemLog(Log_Info, QString(u8"压力/位移传感器初始化完成"));
            VisAppBus::postEvent("PressDispSensorReady");
        }
        // 托盘RFID自动连接
        auto& rfidParam = GlobalParam->hardwareParam.trayRfidDebugParam;
        TrayRfidManager::instance()->addRfid(FeedPCB);
        TrayRfidManager::instance()->setRfidParm(rfidParam.comPort, rfidParam.StrbaudRate.toInt());
        bool bRfidRet = TrayRfidManager::instance()->connect();
        if (bRfidRet) {
            ShowSystemLog(Log_Info, QString(u8"%1RFID读码器自动连接成功").arg(rfidParam.comPort));
            VisAppBus::postEvent("ScanTrayReady");
        }
        else {
            ShowSystemLog(Log_Error, QString(u8"%1RFID读码器自动连接失败").arg(rfidParam.comPort));
        }
    }
    return 0;
}

void PluginDevice::ReConnetCam(bool checkState)
{
    if (GlobalParam->frameCore->curUserInfo.authority == OPERATOR) {
        QMessageBox::warning(nullptr, QString(u8"警告"), QString(u8"当前用户无权限"));
        return;
    }
    GlobalThreadPool->Commit_Topic("InitDevice", std::bind(&PluginDevice::InitDevice, this));
    GlobalThreadPool->WaitTask("InitDevice");
}

int PluginDevice::InitDevice()
{
    int nRet = 0;
    // 1、初始化工业相机
    nRet = InitOtherCam();
    if(nRet != 0) return nRet;

    ShowSystemLog(Log_Info, QString(u8"工业相机初始化完成"));
    return nRet;
}

int PluginDevice::InitOtherCam(bool open)
{
    int nRes = 0;
    QStringList listCam;
    listCam << FeedHolderCam << FeedPCBCam << DirtyDetectCam  << AssembleTopCam  << AssembleBottomCam;

    for (int i = 0; i < listCam.size(); i++) {
        VisCameraTool::instance()->StopStream(0, listCam.at(i));
        VisCameraTool::instance()->CloseCamera(0, listCam.at(i));
        if (!open) continue;
        //快速重启软件时相机驱动资源未释放/网络未注册,打开失败:
        //重试5次每次间隔2秒,重试前先枚举设备+CloseCamera清残留句柄,枚举到设备才尝试打开
        nRes = -1;
        for (int nTry = 0; nTry < 3; nTry++) {
            std::vector<std::string> devInfo;
            VisCameraTool::instance()->EnumCamera(listCam.at(i), devInfo);
            if (!devInfo.empty()) {
                nRes = VisCameraTool::instance()->OpenCamera(0, listCam.at(i));
                if (nRes == 0)break;
                ShowSystemLog(Log_Info, QString(u8"%1相机打开失败:%2,第%3次重试").arg(listCam.at(i)).arg(nRes).arg(nTry + 1));
                VisCameraTool::instance()->CloseCamera(0, listCam.at(i));
            }
            else {
                ShowSystemLog(Log_Info, QString(u8"%1设备未就绪(枚举为空),第%2次等待").arg(listCam.at(i)).arg(nTry + 1));
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(2000));
        }
        if (nRes != 0) {
            ShowSystemLog(Log_Error, listCam.at(i) + QString(u8"相机打开失败:%1").arg(nRes));
            return nRes;
        }
        nRes = VisCameraTool::instance()->StartStream(0, listCam.at(i), false);
        if (nRes != 0) {
            ShowSystemLog(Log_Error, listCam.at(i) + QString(u8"相机开始出流失败:%1").arg(nRes));
            return nRes;
        }
    }
    ShowSystemLog(Log_Info, QString(u8"工业相机初始化成功"));
    m_connectFlag = open;
    return 0;
}

//==================== 压力传感器初始化 ====================
int PluginDevice::InitPressureSensor()
{
    PressureSensorSettings pressSet;
    // 上电初始化读取调试参数容器内默认参数
    auto& debugParam = GlobalParam->hardwareParam.pressDispDebugParam;
    pressSet.PortName = debugParam.StrpressPort;
    pressSet.BaudRate = debugParam.StrpressBaud.toInt();
    pressSet.DataBits = debugParam.pressDataBits;
    pressSet.Parity = debugParam.pressParity;
    pressSet.StopBits = debugParam.pressStopBits;
    pressSet.ResponseTime = debugParam.pressRespTime;
    pressSet.NumberOfRetries = debugParam.pressRetry;
    pressSet.SlaveID = debugParam.pressSlaveID;

    int ret = m_pressureClient.Connect(pressSet);
    if(ret != 0)
    {
        ShowSystemLog(Log_Error, QString(u8"%1 压力传感器初始化失败").arg(m_stationName));
        return -1;
    }
    ShowSystemLog(Log_Info, QString(u8"%1 压力传感器初始化成功").arg(m_stationName));
    return 0;
}

//==================== 位移传感器初始化 ====================
int PluginDevice::InitDisplacementSensor()
{
    DisplacementPortSettings laserSet;
    auto& debugParam = GlobalParam->hardwareParam.pressDispDebugParam;
    laserSet.PortName = debugParam.StrdispPort;
    laserSet.BaudRate = debugParam.StrdispBaud.toInt();
    laserSet.DataBits = debugParam.dispDataBits;
    laserSet.Parity = debugParam.dispParity;
    laserSet.StopBits = debugParam.dispStopBits;

    int ret = m_displacementClient.Connect(laserSet);
    if(ret != 0)
    {
        ShowSystemLog(Log_Error, QString(u8"%1 位移传感器初始化失败").arg(m_stationName));
        return -1;
    }
    ShowSystemLog(Log_Info, QString(u8"%1 位移传感器初始化成功").arg(m_stationName));
    return 0;
}

PressureSensorSettings PluginDevice::VariantMapToPressSetting(const QVariantMap& map)
{
    PressureSensorSettings s;
    s.PortName = map["PortName"].toString();
    s.BaudRate = map["BaudRate"].toInt();
    s.DataBits = map["DataBits"].toInt();
    s.Parity = map["Parity"].toInt();
    s.StopBits = map["StopBits"].toInt();
    s.SlaveID = map["SlaveID"].toInt();
    s.ResponseTime = 1000;
    s.NumberOfRetries = 1;
    return s;
}

DisplacementPortSettings PluginDevice::VariantMapToDispSetting(const QVariantMap& map)
{
    DisplacementPortSettings s;
    s.PortName = map["PortName"].toString();
    s.BaudRate = map["BaudRate"].toInt();
    s.DataBits = map["DataBits"].toInt();
    s.Parity = map["Parity"].toInt();
    s.StopBits = map["StopBits"].toInt();
    ShowSystemLog(Log_Info, QString(u8"位移传感器参数:Port=%1,Baud=%2,Data=%3,Parity=%4,Stop=%5")
        .arg(s.PortName).arg(s.BaudRate).arg(s.DataBits).arg(s.Parity).arg(s.StopBits));
    return s;
}

int PluginDevice::event_ScanCodeConnect(QString name)
{
    if(name == ScanHolderCode)
        return m_scanCodeTray.Connect(GlobalParam->hardwareParam.trayCodeParam.sIP);
    if(name == ScanPCBCode)
        return m_scanCodePCB.Connect(GlobalParam->hardwareParam.pcbCodeParam.sIP);
    return 0;
}

int PluginDevice::event_DisScanCodeConnect(QString name)
{
    if(name == ScanHolderCode)
        return m_scanCodeTray.Disconnected();
    if(name == ScanPCBCode)
        return m_scanCodePCB.Disconnected();
    return 0;
}

int PluginDevice::event_PressureSensorConnect(QString name, QVariant param)
{
    if(m_stationName != name)
        return 0;
    if(!param.canConvert<QVariantMap>())
        return -1;
    QVariantMap map = param.toMap();
    PressureSensorSettings setting = VariantMapToPressSetting(map);
    return m_pressureClient.Connect(setting);
}

int PluginDevice::event_PressureSensorDisconnect(QString name)
{
    if(m_stationName != name) return 0;
    return m_pressureClient.Disconnected();
}

int PluginDevice::event_PressureSensorGetRealPressure(QString name, float &pressure)
{
    if(m_stationName != name)
        return -2;
    int ret = m_pressureClient.ReadRealPressure();
    if (ret != 0)
    {
        pressure = 0.0f;
        return -1;
    }
    m_pressureClient.GetRealTimePressure(pressure);
    return 0;
}

int PluginDevice::event_PressureSensorGetPeakPressure(QString name, float &pressure)
{
    if(m_stationName != name)
        return -2;
    int ret = m_pressureClient.ReadPeakPressure();
    if (ret != 0)
    {
        pressure = 0.0f;
        return -1;
    }
    m_pressureClient.GetPeakPressure(pressure);
    return 0;
}

int PluginDevice::event_PressureSensorResetPeak(QString name)
{
    if(m_stationName != name) return 0;
    return m_pressureClient.ResetPeakValley();

}

int PluginDevice::event_DisplacementSensorConnect(QString name, QVariant param)
{
    if(m_stationName != name)
        return 0;
    if (!param.canConvert<QVariantMap>())
    {
        ShowSystemLog(Log_PipeLine, QString(u8"%1 位移传感器连接参数格式错误").arg(m_stationName));
        return -1;
    }
    QVariantMap map = param.toMap();
    DisplacementPortSettings laserSet = VariantMapToDispSetting(map);
    int ret = m_displacementClient.Connect(laserSet);
    if(ret != 0)
    {
        ShowSystemLog(Log_Error, QString(u8"%1 位移传感器连接失败").arg(m_stationName));
    }
    else
    {
        ShowSystemLog(Log_PipeLine, QString(u8"%1 位移传感器连接成功").arg(m_stationName));
    }
    return ret;
}
int PluginDevice::event_DisplacementSensorDisconnect(QString name)
{
    if(m_stationName != name) return 0;
    return m_displacementClient.DisconnectPort();
}

int PluginDevice::event_DisplacementSensorReadHeight(QString name, QString& sensorId,double &height)
{
    if(m_stationName != name)
        return 0;

    double heightVal = 0.0;
    int ret = m_displacementClient.ReadSensorHeight(sensorId, heightVal);
    height = heightVal;
    return ret;
}

int PluginDevice::event_DisplacementSensorResetZero(QString name)
{
    if(m_stationName != name)
        return 0;
    return m_displacementClient.ResetSensorZero();
}

int PluginDevice::event_ConnectModbus(QString port, int baud)
{
    TrayRfidManager::instance()->addRfid(FeedPCB);
    bool bRet = TrayRfidManager::instance()->setRfidParm(port,baud);
    bRet = TrayRfidManager::instance()->connect();
    ShowSystemLog(bRet ? Log_Info : Log_Error, QString(u8"%1RFID读码器初始化%2！").arg(port).arg(bRet ? u8"成功" : u8"失败"));
    return bRet?0:1;
}

int PluginDevice::event_DisconnectedModbus()
{
    bool bRet = TrayRfidManager::instance()->disConnect();
    return bRet?0:1;
}

int PluginDevice::event_GetModbusTrayCode(TrayFunc num, QString &barCode, int length)
{
    bool bRet = TrayRfidManager::instance()->GetModbusTrayCode(num,barCode,length);
    return bRet?0:1;
}

int PluginDevice::event_GetModbusRFIDExist(int num, bool &exist)
{
    bool bRet = TrayRfidManager::instance()->GetModbusRFIDExist(num,exist);
    return bRet?0:1;
}

int PluginDevice::event_SetModbusModel(TrayFunc num)
{
    bool bRet = TrayRfidManager::instance()->SetModbusModel(num);
    return bRet?0:1;
}

int PluginDevice::event_SetModbusTrayCode(TrayFunc num, QString &barCode)
{
    bool bRet = TrayRfidManager::instance()->SetModbusTrayCode(num,barCode);
    return bRet?0:1;
}

int PluginDevice::event_TriggerReadCard(TrayFunc num)
{
    bool bRet = TrayRfidManager::instance()->TriggerReadCard(num);
    return bRet ? 0 : 1;
}

int PluginDevice::event_ReadRfidTag(TrayFunc num, QString &outBarCode)
{
    bool bRet = TrayRfidManager::instance()->ReadRfidTag(num, outBarCode);
    return bRet ? 0 : 1;
}

