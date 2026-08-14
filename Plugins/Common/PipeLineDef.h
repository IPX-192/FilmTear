#ifndef PIPELINEDEF_H
#define PIPELINEDEF_H

#include <QObject>
#include <QDataStream>

#define  UdpClientPort  8080
#define  UdpServerPort  8989

//enum PipeLineStatus{
//    PipeLine_PreToCur,   //上一站到当前站
//    PipeLine_Load,       //入料口到测试位
//    PipeLine_Blank,      //测试位下料到出料口
//    PipeLine_CurToNext,  //当前站到下一站
//};

// 上游联机 Client 8路状态
struct UpStreamClientStatus
{
    //本站3个
    bool  m_trayRequest=false;             //输送线本站要料
    bool  m_trayInCurPos = false;          //输送线上站托盘到达本站
    bool  m_existTrayBackFlow = false;     //本站回流线有盘
    //上游3个
    bool  m_preExistTray=false;             //上站有料
    bool  m_preRequestBackFlow = false;     //上站回流线要料
    bool  m_trayInNextPosBackFlow = false;  //回流线托盘到达上站
    //传输2个
    bool  m_transTray=false;               //输送线传输托盘
    bool  m_transTrayBackFlow = false;     //回流托盘传输上站状态
};

// 下游联机 Server 8路状态
struct DownStreamServerStatus
{
    //本站3个
    bool  m_existTray=false;              //本站流线出口有盘
    bool  m_requestTrayBackFlow = false;  //本站回流线要料
    bool  m_trayToBackFlow = false;       //回流线托盘到达本站
    //下游3个
    bool  m_nextRequest=false;             //下游请求托盘
    bool  m_nextBackFlowExist = false;     //下游回流线有盘
    bool  m_trayInNextPos =false;          //输送线托盘到达下一站
    //传输2个
    bool  m_transTray = false;           //输送线传输托盘状态
    bool  m_transTrayBackFlow=false;     //回流线传输托盘状态
};

#endif // PIPELINEDEF_H
