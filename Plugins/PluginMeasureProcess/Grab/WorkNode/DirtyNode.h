#ifndef DIRTYNODE_H
#define DIRTYNODE_H

#include <QObject>
#include "ParamDef.h"

#include "VisAnomalyDetect_API.h"


class DirtyNode
{

public:
     DirtyNode(ModuleInfo* item,int station);

public:
    int Process();

    static int  InitDetector();
    static void ReleaseDetector();
    static int  event_TestDetect(int station);
    static EAD_Handle s_detector;
    static bool       s_initialized;


protected:
    ModuleInfo* m_item = nullptr;
    int  m_station = 0;

protected:
    void  sigShowDirtyImg(int station, QImage imgShow);
};

#endif // DIRTYNODE_H
