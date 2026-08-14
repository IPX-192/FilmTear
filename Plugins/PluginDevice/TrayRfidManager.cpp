#include "TrayRfidManager.h"
#include <QTime>
#include <QThread>
#include <QDomDocument>
#include <QDomElement>
#include <QDomNodeList>
#include <QFileDialog>
#include <QElapsedTimer>
#include <QModbusDevice>
#include "VisAppBus.h"
#include "ParamManager.h"
#include <QModbusRtuSerialMaster>

SINGLETON_IMPL(TrayRfidManager)
TrayRfidManager::TrayRfidManager(QObject *parent) : QObject(parent)
{
	m_modBusParams.parity = QSerialPort::NoParity;
	m_modBusParams.baud = QSerialPort::Baud115200;
	m_modBusParams.dataBits = QSerialPort::Data8;
	m_modBusParams.stopBits = QSerialPort::OneStop;
    m_modBusParams.responseTime = 700;
	m_modBusParams.port = QStringLiteral("COM10");
	m_device = new QModbusRtuSerialMaster(this);
}

bool TrayRfidManager::connect()
{
    disConnect();
	if (m_bConnectState)return true;
	m_device->setConnectionParameter(QModbusDevice::SerialPortNameParameter, m_modBusParams.port);
	m_device->setConnectionParameter(QModbusDevice::SerialParityParameter, m_modBusParams.parity);
	m_device->setConnectionParameter(QModbusDevice::SerialDataBitsParameter, m_modBusParams.dataBits);
	m_device->setConnectionParameter(QModbusDevice::SerialStopBitsParameter, m_modBusParams.stopBits);
	m_device->setConnectionParameter(QModbusDevice::SerialBaudRateParameter, m_modBusParams.baud);
	m_device->setTimeout(m_modBusParams.responseTime);
	m_device->setNumberOfRetries(m_modBusParams.numberOfRetries);

//	QObject::connect(m_device, &QModbusDevice::errorOccurred, this, [this](QModbusDevice::Error) {
//		disConnect();
//	}, Qt::QueuedConnection);

	m_bConnectState = m_device->connectDevice();
	return m_bConnectState;
}

bool TrayRfidManager::disConnect()
{
	if (m_device == nullptr) return false;
	m_device->disconnect();
	m_device->disconnectDevice();
	m_bConnectState = false;

	return true;
}

bool TrayRfidManager::addRfid(int num)
{
    //添加序号上如果没有RFID则添加
    if(!m_allRfidCtrl.keys().contains(num)){
        CModbusRFIDClient *rfid = new CModbusRFIDClient(num,m_device,this);
        m_allRfidCtrl[num]= rfid;
    }
    else{
        m_errInfo=QStringLiteral("当前存在RFID读码器");
    }
    return true;
}

bool TrayRfidManager::removeRfid(int num)
{
    //判断序号上是否有RFID可以删除
    if(m_allRfidCtrl.keys().contains(num)){
        m_allRfidCtrl.remove(num);
    }
    else{
        m_errInfo=QStringLiteral("当前不存在RFID读码器");
    }
    return true;
}

bool TrayRfidManager::setRfidParm(QString comName,int baud)
{
    m_modBusParams.port = comName.toUpper();
    m_modBusParams.baud = baud;
    return true;
}


QString TrayRfidManager::readCurError()
{
    return m_errInfo;
}

bool TrayRfidManager::GetConnnectState()
{
    return m_bConnectState;
}

bool TrayRfidManager::SetModbusModel(int num)
{
    if(m_allRfidCtrl.keys().contains(num)){
        if(!m_allRfidCtrl[num]->SetModbusModel()){
            m_errInfo=m_allRfidCtrl[num]->readCurError();
            return false;
        }
    }
    else{
        m_errInfo=QStringLiteral("当前不存在RFID读码器");
        return false;

    }
    return true;
}

bool TrayRfidManager::GetModbusRFIDExist(int num,bool &exist)
{
    if(m_allRfidCtrl.keys().contains(num)){
        if(!m_allRfidCtrl[num]->GetModbusRFIDExist(exist)){
            m_errInfo=m_allRfidCtrl[num]->readCurError();
            return false;
        }
    }
    else{
        m_errInfo=QStringLiteral("当前不存在RFID读码器,reqNum:%1,existKeys:%2")
                      .arg(num).arg([this](){QStringList l;for(auto k:m_allRfidCtrl.keys())l<<QString::number(k);return l.join(',');}());
        return false;

    }
    return true;
}

bool TrayRfidManager::GetModbusTrayCode(int num,QString &barCode, int length)
{
    if(m_allRfidCtrl.keys().contains(num)){
        if(!m_allRfidCtrl[num]->GetModbusTrayCode(barCode,length)){
            m_errInfo=m_allRfidCtrl[num]->readCurError();
            return false;
        }
    }
    else{
        m_errInfo=QStringLiteral("当前不存在RFID读码器,reqNum:%1,existKeys:%2")
                      .arg(num).arg([this](){QStringList l;for(auto k:m_allRfidCtrl.keys())l<<QString::number(k);return l.join(',');}());
        return false;

    }
    return true;
}

bool TrayRfidManager::SetModbusTrayCode(int num,QString &barCode)
{
    if(m_allRfidCtrl.keys().contains(num)){
        if(!m_allRfidCtrl[num]->SetModbusTrayCode(barCode)){
            m_errInfo=m_allRfidCtrl[num]->readCurError();
            return false;
        }
    }
    else{
        m_errInfo=QStringLiteral("当前不存在RFID读码器");
        return false;

    }
    return true;
}

bool TrayRfidManager::TriggerReadCard(int num)
{
    if(m_allRfidCtrl.keys().contains(num))
    {
        if(!m_allRfidCtrl[num]->TriggerReadCard())
        {
            m_errInfo = m_allRfidCtrl[num]->readCurError();

            ShowSystemLog(Log_Error,QString::fromUtf8("TriggerReadCard失败，num=%1，err=%2").arg(num).arg(m_errInfo));
            return false;
        }
    }
    else
    {
        m_errInfo = QStringLiteral("当前不存在RFID读码器");
        ShowSystemLog(Log_Error,QString::fromUtf8("TriggerReadCard：%1").arg(m_errInfo));
        return false;
    }
    return true;
}


bool TrayRfidManager::ReadRfidTag(int num, QString &outBarCode, int timeoutMs)
{
    outBarCode.clear();
    if(!TriggerReadCard(num))
    {
        return false;
    }

    QThread::msleep(timeoutMs);

    bool exist = false;
    GetModbusRFIDExist(num, exist);

    // 直接读取条码
    bool codeOk = GetModbusTrayCode(num, outBarCode,32);
    QString realCode = outBarCode.trimmed();

    // 条码寄存器有有效数据，就算成功，忽略exist标志清零
    if(codeOk && !realCode.isEmpty())
    {
        outBarCode = realCode;
        return true;
    }

    // 条码为空，才判断是否没有标签
    if(!exist)
    {
        m_errInfo = QStringLiteral("未检测到RFID标签");
        return false;
    }

    m_errInfo = QStringLiteral("读取条码数据失败");
    return false;
}

