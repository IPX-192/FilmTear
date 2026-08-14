#ifndef SCANCODECLIENT_H
#define SCANCODECLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <thread>

class ScanCodeClient : public QObject
{
    Q_OBJECT
public:
    explicit ScanCodeClient(QObject *parent = nullptr);

public:
    void SetName(QString name);
	int Connect(QString ipAddress);
	int Disconnected();
protected:
    void Init();
    bool WaitTime();

protected:
    QString m_name;
    std::thread::id    m_initThreadID;
    QTcpSocket *m_tcpClient = nullptr;              //当前连接的客户端
    bool   m_connectFlag = false;
    bool   m_replyFlag=false;
    int    m_scanCodeMax=3;
    QString m_barCode;

signals:

public slots:
    int  event_Connect(QString name, QString ipAddress);
    int  event_Disconnected(QString name);
    int  event_GetBarCode(QString name, QString& barCode);

protected slots:
    bool slotSendData(QString str);
    void ReadData();
};

#endif // SCANCODECLIENT_H
