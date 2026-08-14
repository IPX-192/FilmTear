#include "LightSourceManager.h"
#include "VisAppBus.h"

SINGLETON_IMPL(LightSourceManager)
LightSourceManager::LightSourceManager()
{
    VisAppBus::subscibeEvent(this, "InitLightPort");
    VisAppBus::subscibeEvent(this, "UnInitLightPortAll");
    VisAppBus::subscibeEvent(this, "UnInitLightPort");
    VisAppBus::subscibeEvent(this, "SetLightValue");
    VisAppBus::subscibeEvent(this, "TurnOffLight");
}

LightSourceManager::~LightSourceManager()
{
}

bool LightSourceManager::InitPort(QString portName, int serBaudRate)
{
    if (portName.isEmpty())
        return false;

    if (!m_mapClient.contains(portName))
    {
        LightSourceClient *client = new LightSourceClient;
        connect(client, SIGNAL(sigRecvData(QString,QByteArray)),
                this, SLOT(slotRecvData(QString,QByteArray)));
        bool isOk = client->InitPort(portName, serBaudRate);
        if (isOk)
            m_mapClient.insert(portName, client);
        else
            delete client;
        return isOk;
    }
    return m_mapClient.value(portName)->IsConnected();
}

bool LightSourceManager::UnInitPortAll()
{
    if (m_mapClient.isEmpty())
        return false;
    for (auto it = m_mapClient.begin(); it != m_mapClient.end(); ++it)
        it.value()->UnInitPort();
    qDeleteAll(m_mapClient);
    m_mapClient.clear();
    return true;
}

bool LightSourceManager::UnInitPort(QString portName)
{
    if (m_mapClient.isEmpty() || !m_mapClient.contains(portName))
        return false;
    m_mapClient.value(portName)->UnInitPort();
    m_mapClient.remove(portName);
    return true;
}

QStringList LightSourceManager::GetPortNameList()
{
    QList<QSerialPortInfo> list = QSerialPortInfo::availablePorts();
    QStringList listPortName;
    for (int i = 0; i < list.size(); i++)
        listPortName << list.at(i).portName();
    return listPortName;
}

bool LightSourceManager::SendCmd(QString portName, QByteArray data)
{
    if (portName.isEmpty() || !m_mapClient.contains(portName))
        return false;
    return m_mapClient.value(portName)->SendCmd(data);
}
bool LightSourceManager::SendCmd(QString portName, QByteArray data, QByteArray &resMsg)
{
    if (portName.isEmpty() || !m_mapClient.contains(portName))
        return false;
    return m_mapClient.value(portName)->SendCmd(data, resMsg);
}

void LightSourceManager::slotRecvData(QString name, QByteArray data)
{
    emit sigResMsg(name, data);
}

int LightSourceManager::event_InitLightPort(QString portName, int serBaudRate)
{
    bool isOk = InitPort(portName, serBaudRate);
    return isOk ? 0 : 1;
}

int LightSourceManager::event_UnInitLightPortAll()
{
    bool isOk = UnInitPortAll();
    return isOk ? 0 : 1;
}

int LightSourceManager::event_UnInitLightPort(QString portName)
{
    bool isOk = UnInitPort(portName);
    return isOk ? 0 : 1;
}

int LightSourceManager::event_SetLightValue(QString portName, int channel, int brightness)
{
    if (channel < 1 || channel > 26 || brightness < 0 || brightness > 255)
        return -1;
    char ch = 'A' + static_cast<char>(channel - 1);
    QString cmd = QString("S%1%2#").arg(ch).arg(brightness, 4, 10, QChar('0'));
    bool isOk = SendCmd(portName, cmd.toLatin1());
    return isOk ? 0 : 1;
}

int LightSourceManager::event_TurnOffLight(QString portName, int channel)
{
    return event_SetLightValue(portName, channel, 0);
}
