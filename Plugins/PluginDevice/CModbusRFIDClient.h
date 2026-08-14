#ifndef CMODBUSRFIDCLIENT_H
#define CMODBUSRFIDCLIENT_H

#include <QObject>
#include <QModbusDataUnit>
#include <QSerialPort>
#include <thread>
#include <QByteArray>
#include <mutex>
#include <QMutex>

class QModbusClient;
class QModbusReply;
class CModbusRFIDClient : public QObject
{
    Q_OBJECT
public:
    struct Settings
    {
        QString port = "";
        int parity = QSerialPort::NoParity;
        int baud = QSerialPort::Baud115200;
        int dataBits = QSerialPort::Data8;
        int stopBits = QSerialPort::OneStop;
        int responseTime = 10000;
        int numberOfRetries = 3;
    };
public:
    explicit CModbusRFIDClient(int serverAddress, QModbusClient *device,QObject *parent = nullptr);
    ~CModbusRFIDClient();

public:
    bool  SetModbusModel();
    bool  GetModbusRFIDExist(bool &exist);
    bool  GetModbusTrayCode(QString& barCode,int length);
    bool  SetModbusTrayCode(QString& barCode);
    bool  TriggerReadCard();
    //读取当前错误信息
    QString readCurError();
protected slots:
    bool slotSendData(QByteArray& sendData, QByteArray& recvData, bool& replyFlag);
public:
    void SetParams(QString port,int bauds);            //设置modbus参数
private:
    bool SendData(QByteArray &sendData, QByteArray &recvData, bool &replyFlag);
    bool WaitTime(bool &replyFlag);

    QByteArray SwapOrder(QByteArray in);
    quint16 ByteArrayToUInt16(const QByteArray &ba, bool littleEndian = false);
    void appendCRC(QByteArray &data);
private:
    QModbusClient *m_device = nullptr;
    quint8 m_serverAddress = 0x02;                 //从机的地址

    std::thread::id    m_initThreadID;
    QMap<QModbusReply *, bool>m_replyTimeout;
    QMap<QModbusReply *, QByteArray>m_replyData;
    QString m_errInfo;
    static std::mutex m_mutexSendData;
    QMutex m_mtxReplyMap;
};

#endif
