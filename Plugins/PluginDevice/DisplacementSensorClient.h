#ifndef DISPLACEMENTSENSORCLIENT_H
#define DISPLACEMENTSENSORCLIENT_H

#include <QObject>
#include <QSerialPort>
#include <QByteArray>
#include <QVector>
#include <thread>
#include <QMutex>
#include <QString>

struct DisplacementPortSettings
{
    QString PortName = "";
    int BaudRate = 9600;
    int DataBits = 8;
    int Parity = 0;    // 0无校验
    int StopBits = 1;
    int TimeoutMs = 3000;
};

class DisplacementSensorClient : public QObject
{
    Q_OBJECT
public:
    explicit DisplacementSensorClient(QObject *parent = nullptr);
    ~DisplacementSensorClient() override;

public:
    void SetName(QString name);
    QStringList ScanAllSerialPort();
    int Connect(const DisplacementPortSettings& settings);
    int DisconnectPort();
    bool IsPortOpened();

    int ReadSensorHeight(const QString& sensorId, double& outHeight);
    int ResetSensorZero();
    int GetLatestRawHeight(double& outVal);

protected:
    bool WaitReply(int timeoutMs);

protected slots:
    bool slotSendAsciiCmd(const QString& cmd);
    void slotReadSerialData();
    void slotSerialPortError(QSerialPort::SerialPortError err);

protected:
    std::thread::id m_initThreadID;
    QString m_name;
    QSerialPort* m_serial = nullptr;
    QMutex m_opMutex;
    QByteArray m_recvBuffer;
    bool m_replyFlag = false;
    QString m_lastErrMsg = "No Error";

    // 位移业务缓存
    double m_currentHeight = 0.0;
    QVector<double> m_curveHistory;
    const int MAX_CURVE_POINTS = 100;

    // 一阶滤波
    double m_lastFilteredVal = 0.0;
    const double FILTER_ALPHA = 0.3;
};

#endif // DISPLACEMENTSENSORCLIENT_H
