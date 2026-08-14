#ifndef PLUGINDEVICE_H
#define PLUGINDEVICE_H

#include "VISFramePluginModel.h"
#include "ScanCodeClient.h"
#include "PressureSensorClient.h"
#include "DisplacementSensorClient.h"
#include "TrayRfidManager.h"
#include "ParamDef.h"
#include <QVariantMap>

class PluginDevice:public VISFramePluginModel
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID Plugin_Interface_iid FILE "QPlugin.json")
    Q_INTERFACES(Plugin_Interface)
public:
    PluginDevice();
    ~PluginDevice();

public:
    virtual void InitActionList(Plugin_Interface* plugin);
    virtual int  OnInitialized();
    virtual int  OnViewClosing();   //主窗口关闭时释放相机资源

protected:
    void  ReConnetCam(bool checkState);

protected:
    int  InitDevice();
    int  InitOtherCam(bool open = true);
    int  InitPressureSensor();
    int  InitDisplacementSensor();

public slots:
    int  event_ScanCodeConnect(QString name);
    int  event_DisScanCodeConnect(QString name);

    int event_PressureSensorConnect(QString name, QVariant param);
    int event_PressureSensorDisconnect(QString name);
    int event_PressureSensorGetRealPressure(QString name, float &pressure);
    int event_PressureSensorGetPeakPressure(QString name, float &pressure);
    int event_PressureSensorResetPeak(QString name);

    int event_DisplacementSensorConnect(QString name, QVariant param);
    int event_DisplacementSensorDisconnect(QString name);
    int event_DisplacementSensorReadHeight(QString name, QString &sensorId, double &height);
    int event_DisplacementSensorResetZero(QString name);

    //托盘Modbus RFID
    int event_ConnectModbus(QString port,int baud);
    int event_DisconnectedModbus();
    int event_GetModbusTrayCode(TrayFunc num,QString& barCode,int length);
    int event_GetModbusRFIDExist(int num,bool &exist);
    int event_SetModbusModel(TrayFunc num);
    int event_SetModbusTrayCode(TrayFunc num,QString& barCode);
    int event_TriggerReadCard(TrayFunc num);
    int event_ReadRfidTag(TrayFunc num, QString &outBarCode);


private:

    PressureSensorSettings VariantMapToPressSetting(const QVariantMap& map);
    DisplacementPortSettings VariantMapToDispSetting(const QVariantMap& map);

    ScanCodeClient m_scanCodeTray;
    ScanCodeClient m_scanCodePCB;
    PressureSensorClient m_pressureClient;
    DisplacementSensorClient m_displacementClient;

    bool m_connectFlag = false;
    const QString m_stationName = "TearStation";
};

#endif // PLUGINDEVICE_H
