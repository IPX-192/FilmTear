#include "LightSourceClient.h"
#include <QElapsedTimer>
#include <QCoreApplication>

LightSourceClient::LightSourceClient()
{
}

LightSourceClient::~LightSourceClient()
{
}

bool LightSourceClient::InitPort(QString portName, int serBaudRate, int serDataBit,
                                  int serParity, int serStopBit, int serFlowCtrl)
{
    if (portName.isEmpty())
        return false;

    m_portName = portName;
    m_baudRate = serBaudRate;
    m_dataBit  = serDataBit;
    m_parity   = serParity;
    m_stopBit  = serStopBit;
    m_flowCtrl = serFlowCtrl;

    m_serial = new QSerialPort;
    m_serial->setBaudRate(serBaudRate);
    m_serial->setParity((QSerialPort::Parity)serParity);
    m_serial->setDataBits((QSerialPort::DataBits)serDataBit);
    m_serial->setStopBits((QSerialPort::StopBits)serStopBit);
    m_serial->setFlowControl((QSerialPort::FlowControl)serFlowCtrl);

    if (!OpenPort())
        return false;

    connect(m_serial, SIGNAL(readyRead()), this, SLOT(DataReceived()));
    return true;
}

bool LightSourceClient::UnInitPort()
{
    if (nullptr == m_serial)
        return false;
    if (m_serial->isOpen())
    {
        m_serial->clear();
        m_serial->close();
    }
    delete m_serial;
    m_serial = nullptr;
    m_isConnect = false;
    return true;
}

void LightSourceClient::DataReceived()
{
    if (nullptr == m_serial)
        return;
    QByteArray data = m_serial->readAll();
    if (!data.isEmpty())
        emit sigRecvData(m_portName, data);
    m_serial->flush();
}

bool LightSourceClient::SendCmd(QByteArray data)
{
    if (nullptr == m_serial || !m_serial->isOpen())
        return false;
    m_serial->write(data);
    m_serial->flush();
    return true;
}

bool LightSourceClient::SendCmd(QByteArray data, QByteArray &resMsg)
{
    if (nullptr == m_serial || !m_serial->isOpen())
        return false;

    m_serial->write(data);

    QElapsedTimer timer;
    timer.start();
    const int timeoutMs = 2000;
    while (timer.elapsed() < timeoutMs)
    {
        if (m_serial->waitForReadyRead(100))
        {
            QByteArray newData = m_serial->readAll();
            while (m_serial->waitForReadyRead(10))
                newData += m_serial->readAll();
            if (!newData.isEmpty())
            {
                resMsg = newData;
                m_serial->flush();
                return true;
            }
        }
        QCoreApplication::processEvents();
    }

    m_serial->flush();
    return false;
}

bool LightSourceClient::IsConnected()
{
    return m_isConnect;
}

bool LightSourceClient::OpenPort()
{
    if (nullptr == m_serial)
        return false;
    if (m_serial->isOpen())
    {
        m_serial->clear();
        m_serial->close();
    }
    m_serial->setPortName(m_portName);
    if (!m_serial->open(QIODevice::ReadWrite))
        return false;
    m_isConnect = true;
    return true;
}
