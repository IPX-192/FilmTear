#ifndef UDPCLIENT_H
#define UDPCLIENT_H

#include <QObject>
#include <QUdpSocket>
#include <QTcpSocket>
#include <QTimer>
#include <thread>
#include "../Common/PipeLineDef.h"
#include "ParamDef.h"


class UdpClient : public QObject
{
    Q_OBJECT
public:
    explicit UdpClient(QObject *parent = nullptr);

public:
    bool Start();
protected:
    bool  SendState();
    void  TransTrayToCur(TrayInfo trayInfo);   //上站传输输送线托盘到本站
    void  TransBackFlowTray();                 //本站传输回流线托盘到上站

protected:
    std::thread::id    m_initThreadID;
    QUdpSocket* m_uSocket;
    QByteArray  m_dataBuf;
    QTimer*m_timer;
    QTimer* m_statusNotifyTimer;

    bool  m_trayRequest=false;             //输送线本站要料
    bool  m_trayInCurPos = false;          //输送线上站托盘到达本站
    bool  m_existTrayBackFlow = false;     //本站回流线有盘

    bool  m_preExistTray=false;             //上站有料
    bool  m_preRequestBackFlow = false;     //上站回流线要料
    bool  m_trayInNextPosBackFlow = false;  //回流线托盘到达上站

    bool  m_transTray=false;               //输送线传输托盘
    bool  m_transTrayBackFlow = false;     //回流托盘传输上站状态
  

protected slots:
    void  slotTimeOut();
    void  slotUdpReceive();
    bool  slotPostData();
    void  slotNotifyFullStatus();

protected:
    void  sigPipeLineTrayReady(TrayInfo trayInfo);   //通知流线模块上站托盘到位
    void  sigSetBackFlowStatus(bool exist);          //告知server模块回流线无料

public slots:
    int  event_SetPipeLineInStatus(bool exist);     //流线模块设置输送线要料
    int  event_SetBackFlowReady(bool exist);        //Server模块设置回流线有料
};

#endif // UDPCLIENT_H
