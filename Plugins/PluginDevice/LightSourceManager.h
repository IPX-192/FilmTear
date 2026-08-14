#ifndef LIGHTSOURCEMANAGER_H
#define LIGHTSOURCEMANAGER_H

#include <QObject>
#include <QMap>
#include "LightSourceClient.h"
#include "Singleton.h"

class LightSourceManager : public QObject
{
    Q_OBJECT
    SINGLETON_DECL(LightSourceManager)
public:
    ~LightSourceManager();

    bool InitPort(QString portName, int serBaudRate = 9600);
    bool UnInitPortAll();
    bool UnInitPort(QString portName);

    QStringList GetPortNameList();
    bool SendCmd(QString portName, QByteArray data);
    bool SendCmd(QString portName, QByteArray data, QByteArray &resMsg);

public slots:
    int event_InitLightPort(QString portName, int serBaudRate);
    int event_UnInitLightPortAll();
    int event_UnInitLightPort(QString portName);
    int event_SetLightValue(QString portName, int channel, int brightness);
    int event_TurnOffLight(QString portName, int channel);

signals:
    void sigResMsg(QString, QByteArray);

private slots:
    void slotRecvData(QString, QByteArray);

private:
    explicit LightSourceManager();
    QMap<QString, LightSourceClient*> m_mapClient;
};

#endif // LIGHTSOURCEMANAGER_H
