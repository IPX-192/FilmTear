#ifndef PIPELINEMANAGER_H
#define PIPELINEMANAGER_H

#include <QObject>
#include <mutex>
#include <QMap>
#include <QTimer>
#include "hthread.h"
#include "ParamDef.h"
#include "../Common/PipeLineDef.h"
#include "Grab/ThreadGrabDef.h"

class LoadTray;
class BlankTray;
class UdpClient;
class UdpServer;
class PipeLineManager : public QObject, public HThread
{
    Q_OBJECT
public:
    explicit PipeLineManager(QObject *parent = nullptr);
    ~PipeLineManager();

public:
    int  Init();

protected:
    virtual bool doTask();
    int  MoveTray(TrayFunc src);
    int  MoveTrayToHolder();
    int  MoveTrayToFeedPCB();
    int  MoveTrayToCleanPCB();
    int  MoveTrayToTurntable();

protected:
    std::mutex m_pipeLineMutex;
    int  m_pineLineUseCount[2];    //流水线使用计数 2:左右

    UdpClient*m_udpClient;
    UdpServer*m_udpServer;

    QMap<TrayFunc,QString>m_mapTrayName;
    QMap<TrayFunc,QPair<bool, TrayInfo>>m_mapExistTrayInfo;        //位置是否存在托盘,及托盘信息

    std::mutex              m_mutex;
    std::condition_variable m_condition;
    QVector<TrayFunc>    m_listTrayTask;       //托盘待移动队列,上游 / 回流线有托盘时压入，线程循环取出执行搬运。


protected:
    void  sigPipeLineTrayReady(TrayFunc type, TrayInfo info);     //告知各搬运模块，托盘到位
    void  sigSetPipeLineOutStatus(bool exist, TrayInfo info);     //告知联机模块，出口有托盘
    void  sigSetBackFlowStatus(bool exist);                       //告知联机模块，回流线空托盘流走

public slots:
    int  event_SetPipeLineMove(bool enable);
    int  event_SetBackFlowReady();          //回流线空托盘就绪
    int  event_BlankPipeLineTray(TrayFunc type, TrayInfo info);   //搬运模块下料完成
    int  event_PipeLineOutTransEnd();       //流水线出口托盘流走
    int  event_RefreshAllTrayMap(QVector<TrayFunc>& vecFunc, QVector<bool>& vecHasTray, QVector<TrayInfo>& vecTrayData);
};

#endif // PIPELINEMANAGER_H
