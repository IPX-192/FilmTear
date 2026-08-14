#ifndef TRAYRFIDMANAGER_H
#define TRAYRFIDMANAGER_H

#include <QObject>
#include <QMap>
#include "CModbusRFIDClient.h"
#include "Singleton.h"

class TrayRfidManager : public QObject
{
    Q_OBJECT
    SINGLETON_DECL(TrayRfidManager)
    public:
        explicit TrayRfidManager(QObject *parent = nullptr);
    //添加Rfid
    bool addRfid(int num);
    //删除Rfid
    bool removeRfid(int num);
    //
    bool setRfidParm(QString comName,int baud);
    //连接
    bool connect();
    //断开
    bool disConnect();
    //获取连接状态
    bool GetConnnectState();

    bool  SetModbusModel(int num);
    bool  GetModbusRFIDExist(int num,bool &exist);
    bool  GetModbusTrayCode(int num,QString& barCode,int length);
    bool  SetModbusTrayCode(int num,QString& barCode);
    //读取当前错误信息
    QString readCurError();

    bool TriggerReadCard(int num);
    /// 完整读卡流程：触发读卡 + 等待硬件时间 + 判断标签存在 + 读取条码
    /// 注意：内部有延时，禁止主线程调用，必须子线程执行
    bool ReadRfidTag(int num, QString &outBarCode, int timeoutMs = 200);

signals:

private:
    CModbusRFIDClient::Settings m_modBusParams;
    QModbusClient *m_device = nullptr;
    QMap<int,CModbusRFIDClient*> m_allRfidCtrl;//所有rfid
    QString m_errInfo;
    bool m_bConnectState=false;
};

#endif // TRAYRFIDMANAGER_H
