#include "PressureSensorClient.h"
#include <QTime>
#include <QApplication>
#include "ParamManager.h"

PressureSensorClient::PressureSensorClient(QObject *parent)
    : QObject(parent)
{
    m_initThreadID = std::this_thread::get_id();
    Init();
}

PressureSensorClient::~PressureSensorClient()
{
    if (m_modbusMaster)
    {
        m_modbusMaster->disconnectDevice();
        m_modbusMaster->deleteLater();
        m_modbusMaster = nullptr;
    }
}

void PressureSensorClient::SetName(QString name)
{
    m_name = name;
}

void PressureSensorClient::Init()
{
    m_modbusMaster = new QModbusRtuSerialMaster(this);
    m_modbusMaster->disconnectDevice();
    connect(m_modbusMaster, &QModbusDevice::errorOccurred,
            this, &PressureSensorClient::slotModbusErrorOccurred);
}

int PressureSensorClient::Connect(const PressureSensorSettings& settings)
{
    if (m_connectFlag)
    {
        m_modbusMaster->disconnectDevice();
    }
    //串口参数配置
    m_modbusMaster->setConnectionParameter(QModbusDevice::SerialPortNameParameter, settings.PortName);
    m_modbusMaster->setConnectionParameter(QModbusDevice::SerialBaudRateParameter, settings.BaudRate);
    m_modbusMaster->setConnectionParameter(QModbusDevice::SerialDataBitsParameter, settings.DataBits);
    m_modbusMaster->setConnectionParameter(QModbusDevice::SerialParityParameter, settings.Parity);
    m_modbusMaster->setConnectionParameter(QModbusDevice::SerialStopBitsParameter, settings.StopBits);
    m_modbusMaster->setTimeout(settings.ResponseTime);
    m_modbusMaster->setNumberOfRetries(settings.NumberOfRetries);

    m_slaveId = settings.SlaveID;
    m_connectFlag = m_modbusMaster->connectDevice();
    ShowSystemLog(m_connectFlag ? Log_Info : Log_Error,
                  QString(u8"%1压力传感器Modbus连接%2").arg(m_name).arg(m_connectFlag ? u8"成功" : u8"失败"));
    return m_connectFlag ? 0 : -1;
}

int PressureSensorClient::Disconnected()
{
    m_modbusMaster->disconnectDevice();
    m_connectFlag = false;
    ShowSystemLog(Log_Info, QString(u8"%1压力传感器断开连接").arg(m_name));
    return 0;
}

//==================== 本地同步读取接口（读取缓存，不访问串口） ====================
int PressureSensorClient::GetRealTimePressure(float& realPressure)
{
    realPressure = m_grossValue;
    return m_connectFlag ? 0 : -1;
}

int PressureSensorClient::GetPeakPressure(float& peakPressure)
{
    peakPressure = m_peakValue;
    return m_connectFlag ? 0 : -1;
}

int PressureSensorClient::ReadRealPressure()
{
    if (!m_connectFlag)
        return -1;

    bool bRet = false;
    if (std::this_thread::get_id() == m_initThreadID)
    {
        bRet = slotReadGross();
    }
    else
    {
        QMetaObject::invokeMethod(this, "slotReadGross", Qt::BlockingQueuedConnection,
                                  Q_RETURN_ARG(bool, bRet));
    }
    return bRet ? 0 : -1;
}

int PressureSensorClient::ReadPeakPressure()
{
    if (!m_connectFlag)
        return -1;

    bool bRet = false;
    if (std::this_thread::get_id() == m_initThreadID)
    {
        bRet = slotReadPeak();
    }
    else
    {
        QMetaObject::invokeMethod(this, "slotReadPeak", Qt::BlockingQueuedConnection,
                                  Q_RETURN_ARG(bool, bRet));
    }
    return bRet ? 0 : -1;
}

int PressureSensorClient::ResetPeakValley()
{
    if (!m_connectFlag)
        return -1;

    bool bRet = false;
    if (std::this_thread::get_id() == m_initThreadID)
    {
        bRet = slotResetPeakValley();
    }
    else
    {
        QMetaObject::invokeMethod(this, "slotResetPeakValley", Qt::BlockingQueuedConnection,
                                  Q_RETURN_ARG(bool, bRet));
    }
    return bRet ? 0 : -1;
}

bool PressureSensorClient::slotReadGross()
{
    QMutexLocker locker(&m_opMutex);
    int ret = _ReadInputRegisters(0x0000, 2, m_slaveId);
    if (ret < 0)
        return false;
    return true;
}

bool PressureSensorClient::slotReadPeak()
{
    QMutexLocker locker(&m_opMutex);
    int ret = _ReadInputRegisters(0x0004, 2, m_slaveId);
    if (ret < 0)
        return false;
    return true;
}

bool PressureSensorClient::slotResetPeakValley()
{
    QMutexLocker locker(&m_opMutex);
    QVector<uint16_t> writeData = {0x450E, 0x0000};
    int ret = _RequestRegister(QModbusDataUnit::HoldingRegisters, 0x4604, 2, writeData, m_slaveId);
    if (ret < 0)
        return false;
    return true;
}

void PressureSensorClient::slotModbusErrorOccurred(QModbusDevice::Error error)
{
    if (error == QModbusDevice::NoError)
        return;
    m_lastErrorMsg = QString(u8"Modbus异常：%1").arg(m_modbusMaster->errorString());
    ShowSystemLog(Log_Error, QString(u8"%1 %2").arg(m_name).arg(m_lastErrorMsg));
}

void PressureSensorClient::slotReadResponseDistributor()
{
    QModbusReply* reply = qobject_cast<QModbusReply*>(sender());
    if(!reply) return;
    if (reply->error() == QModbusDevice::NoError)
    {
        int startAddr = reply->result().startAddress();
        if (startAddr == 0x0000)
        {
            _DealGrossResponse(reply);
        }
        else if (startAddr == 0x0004)
        {
            _DealPeakResponse(reply);
        }
    }
    else
    {
        m_lastErrorMsg = QString(u8"读取响应错误：Request timeout.");
        ShowSystemLog(Log_Error, QString(u8"%1 %2").arg(m_name).arg(m_lastErrorMsg));
    }
    reply->deleteLater();
}

//==================== 底层Modbus读写封装 ====================
int PressureSensorClient::_ReadInputRegisters(int startAddr, uint16_t regCount, int slaveId)
{
    return _RequestRegister(QModbusDataUnit::InputRegisters, startAddr, regCount, QVector<uint16_t>(), slaveId);
}

int PressureSensorClient::_RequestRegister(QModbusDataUnit::RegisterType regType, int startAddr,
                                           uint16_t regCount, const QVector<uint16_t>& writeVals, int slaveId)
{
    if(m_modbusMaster->state() != QModbusDevice::ConnectedState)
    {
        m_lastErrorMsg = QString(u8"Modbus未连接");
        return -1;
    }
    QModbusDataUnit dataUnit(regType, startAddr, regCount);
    QModbusReply* reply = nullptr;
    if (regType == QModbusDataUnit::HoldingRegisters && !writeVals.isEmpty())
    {
        dataUnit.setValues(writeVals);
        reply = m_modbusMaster->sendWriteRequest(dataUnit, slaveId);
    }
    else
    {
        reply = m_modbusMaster->sendReadRequest(dataUnit, slaveId);
    }
    if (!reply)
    {
        m_lastErrorMsg = QString(u8"发送Modbus请求失败：%1").arg(m_modbusMaster->errorString());
        return -1;
    }
    if (reply->isFinished())
    {
        // 本地模拟器/快速从站可能同步完成，直接解析
        if (reply->error() == QModbusDevice::NoError)
        {
            if (startAddr == 0x0000)
                _DealGrossResponse(reply);
            else if (startAddr == 0x0004)
                _DealPeakResponse(reply);
        }
        reply->deleteLater();
        return 0;
    }
    // 等待异步回复完成，确保调用方立刻能读到值
    QTime timer;
    timer.start();
    while (!reply->isFinished() && timer.elapsed() < 3000)
    {
        QApplication::processEvents();
    }
    if (!reply->isFinished())
    {
        m_lastErrorMsg = QString(u8"Modbus读取超时");
        reply->deleteLater();
        return -1;
    }
    if (reply->error() == QModbusDevice::NoError)
    {
        if (startAddr == 0x0000)
            _DealGrossResponse(reply);
        else if (startAddr == 0x0004)
            _DealPeakResponse(reply);
    }
    reply->deleteLater();
    return 0;
}

//==================== 浮点数据解析 ====================
void PressureSensorClient::_DealGrossResponse(QModbusReply* reply)
{
    QVector<uint16_t> regs = reply->result().values();
    if (regs.size() < 2)
    {
        m_lastErrorMsg = u8"读取实时压力：寄存器返回长度不足";
        ShowSystemLog(Log_Error, QString(u8"%1 %2").arg(m_name).arg(m_lastErrorMsg));
        return;
    }

    uint32_t raw = (static_cast<uint32_t>(regs[0]) << 16) | static_cast<uint32_t>(regs[1]);
    memcpy(&m_grossValue, &raw, sizeof(float));

    // 增加调试日志，打印原始寄存器
    ShowSystemLog(Log_Debug, QString("reg0:%1 reg1:%2 rawFloat:%3")
                  .arg(regs[0],4,16)
                  .arg(regs[1],4,16)
                  .arg(m_grossValue));

    ShowSystemLog(Log_Info, QString(u8"%1 实时压力：%2")
                  .arg(m_name).arg(QString::number(static_cast<double>(m_grossValue), 'f', 2)));
}

void PressureSensorClient::_DealPeakResponse(QModbusReply* reply)
{
    QVector<uint16_t> regs = reply->result().values();
    if (regs.size() < 2)
    {
        m_lastErrorMsg = u8"读取峰值压力：寄存器返回长度不足";
        ShowSystemLog(Log_Error, QString(u8"%1 %2").arg(m_name).arg(m_lastErrorMsg));
        return;
    }
    uint32_t raw = (static_cast<uint32_t>(regs[0]) << 16) | static_cast<uint32_t>(regs[1]);
    memcpy(&m_peakValue, &raw, sizeof(float));

    ShowSystemLog(Log_Debug, QString("reg0:%1 reg1:%2 rawFloat:%3")
                  .arg(regs[0],4,16)
                  .arg(regs[1],4,16)
                  .arg(m_peakValue));

    ShowSystemLog(Log_Info, QString(u8"%1 峰值压力：%2")
                  .arg(m_name).arg(QString::number(static_cast<double>(m_peakValue), 'f', 2)));
}
