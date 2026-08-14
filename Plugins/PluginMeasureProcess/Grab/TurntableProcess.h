#ifndef TURNTABLEPROCESS_H
#define TURNTABLEPROCESS_H

#include <QObject>
#include "ThreadGrabDef.h"
#include "hthread.h"
#include "ParamManager.h"
#include "TurntableWorkStep.h"

class TurntableProcess : public QObject, public HThread
{
    Q_OBJECT
public:
    explicit TurntableProcess(QObject *parent = nullptr);
    ~TurntableProcess();

public:
    int   InitParam();

protected:
    virtual bool doTask();
    bool       Process();
    int        RotateTurntable();
    int        WaitBlankAndNewPCB();

protected:
    QVector<TurntableWorkStep*>m_listPCB;
    int        m_curFixture=0;        //当前上料治具
    bool       m_fixtureIdle=false;   //治具空闲
    bool       m_newPcbReady=false;
    bool       m_clearFlag=false;     //清料标志
    bool       m_blankGrabEnd=false;  //下料抓取结束

protected:
    void  sigPCBTestEnd(ModuleInfo item);

public slots:
    int   event_SetClearState();
    int   event_PCBReady(ModuleInfo item);
    int   event_GrabTurntablePCBEnd();
    int   event_GetCurFixtureIsIdle(bool& idle, int& indexFixture);
    int   event_ManualSelectFixture(int jigNo);
};

#endif // TURNTABLEPROCESS_H
