#ifndef UDPSERVER_H
#define UDPSERVER_H

#include <QObject>
#include <QUdpSocket>
#include <QTimer>
#include <thread>
#include "../Common/PipeLineDef.h"
#include "ParamDef.h"

class UdpServer : public QObject
{
    Q_OBJECT
public:
    explicit UdpServer(QObject *parent = nullptr);

	bool  Start();

protected:
    bool  SendState();
    void  TransTrayToNext();       //本站托盘到下游设备
    void  TransTrayToCur();        //下站回流线托盘到本站

protected:
    std::thread::id    m_initThreadID;
    QUdpSocket* m_uSocket;

    QTimer* m_timer = nullptr;
    QTimer* m_statusNotifyTimer;

    //本站信号
    TrayInfo m_trayInfo;
    bool  m_existTray=false;              //本站流线出口有盘
    bool  m_requestTrayBackFlow = false;  //本站回流线要料
    bool  m_trayToBackFlow = false;       //回流线托盘到达本站
    //下游设备信号
    bool  m_nextRequest=false;             //下游请求托盘
    bool  m_nextBackFlowExist = false;     //下游回流线有盘
    bool  m_trayInNextPos =false;          //输送线托盘到达下一站

    bool  m_transTray = false;           //输送线传输托盘状态
    bool  m_transTrayBackFlow=false;     //回流线传输托盘状态


protected slots:
    void  slotTimeOut();
    void  slotUdpReceive();
    bool  slotPostState();
    void  slotNotifyFullStatus();

protected:
    void  sigPipeLineOutTransEnd();           //输送线出口托盘流到下一站
    void  sigSetBackFlowReady(bool exist);    //回流线空托盘就绪

public slots:
    int   event_SetPipeLineOutStatus(bool exist, TrayInfo trayInfo);    //流线模块设置输送线有料
    int   event_SetBackFlowStatus(bool exist);                          //Client模块设置回流线无料

private:
    int test = 0;

};

#endif // UDPSERVER_H
