#include "DisplacementSensorClient.h"
#include <QTime>
#include <QApplication>
#include <QSerialPortInfo>
#include "ParamManager.h"

DisplacementSensorClient::DisplacementSensorClient(QObject *parent)
    : QObject(parent)
{
    m_initThreadID = std::this_thread::get_id();
    m_serial = new QSerialPort(this);
    m_serial->disconnect();
    connect(m_serial, &QSerialPort::readyRead, this, &DisplacementSensorClient::slotReadSerialData);
    connect(m_serial, &QSerialPort::errorOccurred, this, &DisplacementSensorClient::slotSerialPortError);
}

DisplacementSensorClient::~DisplacementSensorClient()
{
    if (m_serial->isOpen())
    {
        m_serial->close();
    }
    m_serial->deleteLater();
    m_serial = nullptr;
}

void DisplacementSensorClient::SetName(QString name)
{
    m_name = name;
}

QStringList DisplacementSensorClient::ScanAllSerialPort()
{
    QStringList ret;
    for (const auto& info : QSerialPortInfo::availablePorts())
    {
        ret << info.portName();
    }
    return ret;
}

int DisplacementSensorClient::Connect(const DisplacementPortSettings& settings)
{
    QMutexLocker locker(&m_opMutex);
    if (m_serial->isOpen())
    {
        m_serial->close();
    }

    m_serial->setPortName(settings.PortName);
    m_serial->setBaudRate(settings.BaudRate);
    m_serial->setDataBits(static_cast<QSerialPort::DataBits>(settings.DataBits));
    m_serial->setStopBits(static_cast<QSerialPort::StopBits>(settings.StopBits));
    switch (settings.Parity)
    {
    case 1: m_serial->setParity(QSerialPort::OddParity); break;
    case 2: m_serial->setParity(QSerialPort::EvenParity); break;
    default: m_serial->setParity(QSerialPort::NoParity); break;
    }
    m_serial->setFlowControl(QSerialPort::NoFlowControl);

    bool openOk = m_serial->open(QIODevice::ReadWrite);
    ShowSystemLog(openOk ? Log_Info : Log_Error,
        QString(u8"%1 位移传感器串口连接%2！").arg(m_name).arg(openOk ? u8"成功" : u8"失败:" + m_serial->errorString()));
    if (openOk)
        m_recvBuffer.clear();
    return openOk ? 0 : -1;
}

int DisplacementSensorClient::DisconnectPort()
{
    QMutexLocker locker(&m_opMutex);
    if (m_serial->isOpen())
    {
        m_serial->close();
        ShowSystemLog(Log_Info, QString(u8"%1 位移传感器串口已断开").arg(m_name));
    }
    return 0;
}

bool DisplacementSensorClient::IsPortOpened()
{
    return m_serial->isOpen();
}

int DisplacementSensorClient::ReadSensorHeight(const QString& sensorId, double& outHeight)
{
    outHeight = 0.0;
    if (!IsPortOpened())
    {
        m_lastErrMsg = u8"串口未打开";
        return -1;
    }
    bool bRet = false;
    QString cmd = QString("SR,%1,000").arg(sensorId);
    if (std::this_thread::get_id() == m_initThreadID)
    {
        bRet = slotSendAsciiCmd(cmd);
    }
    else
    {
        QMetaObject::invokeMethod(this, "slotSendAsciiCmd", Qt::BlockingQueuedConnection,
            Q_RETURN_ARG(bool, bRet), Q_ARG(QString, cmd));
    }
    if (!bRet || !WaitReply(3000))
    {
        return -1;
    }
    outHeight = m_currentHeight;
    return 0;
}

int DisplacementSensorClient::ResetSensorZero()
{
    if (!IsPortOpened())
        return -1;
    bool bRet1, bRet2;
    QString cmd1 = "AW,050,0";
    QString cmd2 = "AW,050,1";
    if (std::this_thread::get_id() == m_initThreadID)
    {
        bRet1 = slotSendAsciiCmd(cmd1);
        bRet2 = slotSendAsciiCmd(cmd2);
    }
    else
    {
        QMetaObject::invokeMethod(this, "slotSendAsciiCmd", Qt::BlockingQueuedConnection, Q_RETURN_ARG(bool, bRet1), Q_ARG(QString, cmd1));
        QMetaObject::invokeMethod(this, "slotSendAsciiCmd", Qt::BlockingQueuedConnection, Q_RETURN_ARG(bool, bRet2), Q_ARG(QString, cmd2));
    }
    return (bRet1 && bRet2) ? 0 : -1;
}

int DisplacementSensorClient::GetLatestRawHeight(double& outVal)
{
    outVal = m_currentHeight;
    return IsPortOpened() ? 0 : -1;
}

bool DisplacementSensorClient::slotSendAsciiCmd(const QString& cmd)
{
    QMutexLocker locker(&m_opMutex);
    m_replyFlag = false;
    if (!m_serial->isOpen())
    {
        m_lastErrMsg = QString(u8"串口未打开，发送指令失败");
        return false;
    }
    QByteArray sendBuf = (cmd + "\r\n").toLocal8Bit();
    qint64 writeLen = m_serial->write(sendBuf);
    m_serial->flush();
    return writeLen == sendBuf.size();
}

void DisplacementSensorClient::slotReadSerialData()
{
    m_recvBuffer.append(m_serial->readAll());
    while (m_recvBuffer.contains("\r\n"))
    {
        int splitPos = m_recvBuffer.indexOf("\r\n");
        QByteArray frameRaw = m_recvBuffer.left(splitPos);
        m_recvBuffer = m_recvBuffer.mid(splitPos + 2);
        QString frameStr = QString::fromLocal8Bit(frameRaw).trimmed();
        if (!frameStr.isEmpty())
        {
            QStringList parts = frameStr.split(",");
            if (parts.size() >= 3)                           // SR,ID,测量值
            {
                bool ok = false;
                double rawVal = parts[2].toDouble(&ok);   //1为id,2为值
                if (ok)
                {
                    double filterOut = FILTER_ALPHA * rawVal + (1 - FILTER_ALPHA) * m_lastFilteredVal;
                    m_lastFilteredVal = filterOut;
                    m_currentHeight = filterOut;
                    m_curveHistory.append(filterOut);
                    if (m_curveHistory.size() > MAX_CURVE_POINTS)
                        m_curveHistory.removeFirst();
                    m_replyFlag = true;
                }
            }
        }
    }
}

void DisplacementSensorClient::slotSerialPortError(QSerialPort::SerialPortError err)
{
    if (err == QSerialPort::NoError) return;
    m_lastErrMsg = QString(u8"位移传感器串口异常：%1").arg(err);
    ShowSystemLog(Log_Error, QString(u8"%1 %2").arg(m_name).arg(m_lastErrMsg));
}

bool DisplacementSensorClient::WaitReply(int timeoutMs)
{
    QTime timer;
    timer.start();
    while (timer.elapsed() < timeoutMs)
    {
        if (m_replyFlag)
        {
            m_replyFlag = false;
            return true;
        }
        QApplication::processEvents();
    }
    m_lastErrMsg = u8"位移传感器读取响应超时";
    return false;
}
