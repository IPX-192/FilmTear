#ifndef THREADCLEAN_H
#define THREADCLEAN_H

#include <QObject>
#include "ThreadGrabDef.h"
#include "ParamDef.h"

class ThreadClean : public QObject
{
    Q_OBJECT
public:
    explicit ThreadClean(QObject *parent = nullptr);

    int   BeginClean();

protected:
    int        GrabPCB();
    int        CleanPCB();
    int        PlacePCB();
    int        SetGripClose(bool close);
    int        CheckGripModuleExist(bool& exist);

protected:
    const TrayFunc m_funcType=PCBClean;
    TrayInfo m_trayInfo;

protected:
    void   sigBlankPipeLineTray(TrayFunc type, TrayInfo info);

public slots:
    int    event_PipeLineTrayReady(TrayFunc type, TrayInfo info);
};

#endif // THREADCLEAN_H
