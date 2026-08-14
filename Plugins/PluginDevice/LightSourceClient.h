#ifndef LIGHTSOURCECLIENT_H
#define LIGHTSOURCECLIENT_H

#include <QObject>
#include <QByteArray>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>

class LightSourceClient : public QObject
{
    Q_OBJECT
public:
    LightSourceClient();
    ~LightSourceClient();

    bool InitPort(QString portName, int serBaudRate = 9600, int serDataBit = 8,
                  int serParity = 0, int serStopBit = 1, int serFlowCtrl = 0);
    bool UnInitPort();

    bool SendCmd(QByteArray data);
    bool SendCmd(QByteArray data, QByteArray &resMsg);

    bool IsConnected();

signals:
    void sigRecvData(QString portName, QByteArray data);

private slots:
    void DataReceived();

private:
    bool OpenPort();

    QSerialPort *m_serial = nullptr;
    QString m_portName = "COM1";
    int m_flowCtrl = 0;
    int m_parity = 0;
    int m_stopBit = 1;
    int m_dataBit = 8;
    int m_baudRate = 9600;
    bool m_isConnect = false;
};

#endif // LIGHTSOURCECLIENT_H
