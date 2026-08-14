#ifndef TURNTABLEGRAB_H
#define TURNTABLEGRAB_H

#include <QObject>
#include "ThreadGrabDef.h"
#include "hthread.h"

class TurntableProcess;
class TurntableGrab : public QObject, public HThread
{
    Q_OBJECT
public:
    explicit TurntableGrab(QObject *parent = nullptr);
    ~TurntableGrab();

public:
    void   InitParam();

protected:
    virtual bool doTask();
    bool       Process();
    int        GrabTrayModule();
    int        WaitFixtureIdle();
    int        GrabTurntablePCB();
    int        PlaceTurntable();
    int        BlankPCBToTray();
    int        SetGripClose(int index, bool close);
    int        CheckGripModuleExist(int index, bool& exist);

protected:
    TurntableProcess*m_turntableProcess=nullptr;
    const TrayFunc m_funcType=FeedTurntable;
    QString   m_errInfo;
    bool      m_waitFeedTray=true;
    QVector<ModuleInfo>    m_vecFeedPCB;       //上料待测试模组
    QMutex              m_mutexBlankPCB;
    QList<ModuleInfo>   m_listPCBBlank;      //待下料抓取模组队列
    QVector<ModuleInfo> m_vecBlankPCB;      //待下料放置模组
    int        m_curIdleFixture = 0;        //当前空闲治具
    uint64     m_trayFeedNum=0;
    uint64     m_numTotalGrab = 0;          //上料抓取个数
    TrayInfo   m_curTrayInfo;               //当前流线载具信息
    bool       m_clearFlag = false;         //清料状态

protected:
    void   sigPCBReady(ModuleInfo info);
    void   sigGrabTurntablePCBEnd();
    void   sigBlankPipeLineTray(TrayFunc type, TrayInfo info);
    void   sigGetCurFixtureIsIdle(bool& idle, int& indexFixture);

public slots:
    int    event_PipeLineTrayReady(TrayFunc type, TrayInfo info);
    int    event_PCBTestEnd(ModuleInfo module);
};

#endif // TURNTABLEGRAB_H
