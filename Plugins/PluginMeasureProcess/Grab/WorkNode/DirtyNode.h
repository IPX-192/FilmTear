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
    static bool       s_initAttempted;   //已尝试过初始化(成功或失败),避免每个料重复重试


protected:
    ModuleInfo* m_item = nullptr;
    int  m_station = 0;

protected:
    void  sigShowDirtyImg(int station, QImage imgShow);
};

#endif // DIRTYNODE_H
