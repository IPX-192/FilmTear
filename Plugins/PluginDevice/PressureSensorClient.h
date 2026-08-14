#ifndef PRESSURESENSORCLIENT_H
#define PRESSURESENSORCLIENT_H

#include <QObject>
#include <QModbusDevice>
#include <QModbusRtuSerialMaster>
#include <QModbusDataUnit>
#include <QModbusReply>
#include <QMutex>
#include <thread>
#include <QString>
#include <QVector>

struct PressureSensorSettings {
    QString PortName = "";
    int Parity = 0;
    int BaudRate = 115200;
    int DataBits = 8;
    int StopBits = 1;
    int ResponseTime = 5000;
    int NumberOfRetries = 5;
    int SlaveID = 1;
};

class PressureSensorClient : public QObject
{
    Q_OBJECT
public:
    explicit PressureSensorClient(QObject *parent = nullptr);
    ~PressureSensorClient() override;

public:
    void SetName(QString name);
    int Connect(const PressureSensorSettings & settings);
    int Disconnected();
    bool IsConnected() const { return m_connectFlag; }

    // 本地同步读取缓存值（不发送串口）
    int GetRealTimePressure(float& realPressure);
    int GetPeakPressure(float& peakPressure);

    // 发起Modbus查询（发送串口指令）
    int ReadRealPressure();
    int ReadPeakPressure();
    int ResetPeakValley();

protected:
    void Init();

protected:
    QString m_name;
    std::thread::id m_initThreadID;
    QModbusRtuSerialMaster* m_modbusMaster = nullptr;
    QMutex m_opMutex;
    bool m_connectFlag = false;
    QString m_lastErrorMsg = "No Error";
    int m_slaveId = 1;

    // 缓存读取结果
    float m_grossValue = 0.0f;
    float m_peakValue = 0.0f;

protected slots:
    bool slotReadGross();
    bool slotReadPeak();
    bool slotResetPeakValley();
    void slotModbusErrorOccurred(QModbusDevice::Error error);
    void slotReadResponseDistributor();

protected:
    //底层Modbus读写封装
    int _ReadInputRegisters(int startAddr, uint16_t regCount, int slaveId);
    int _RequestRegister(QModbusDataUnit::RegisterType regType, int startAddr,
        uint16_t regCount, const QVector<uint16_t>& writeVals, int slaveId);

    //响应解析处理
    void _DealGrossResponse(QModbusReply* reply);
    void _DealPeakResponse(QModbusReply* reply);
};

#endif // PRESSURESENSORCLIENT_H
