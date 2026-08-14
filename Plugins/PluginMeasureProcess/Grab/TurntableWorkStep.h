#ifndef TURNTABLEWORKSTEP_H
#define TURNTABLEWORKSTEP_H

#include <QObject>
#include "ParamManager.h"

class FilmTearNode;
class PCBCleanNode;
class DirtyNode;
class TurntableWorkStep
{
public:
    TurntableWorkStep(ModuleInfo item);
    TurntableWorkStep &operator =(const TurntableWorkStep&other);

    enum WorkStep{
        FeedStep,      //上料
        FilmTearStep,  //撕膜
        CleanStep,     //清洗
        DirtyStep,     //脏污
        BlankStep,     //下料
        EndStep,       //结束
    };

    int   PreWork();       //上料预处理
    int   Process();
    bool  IsStepBegin(){return m_nextStep ==FeedStep;}
    ModuleInfo GetModuleInfo();

protected:
    int   ProcessBlank();

public:
    int       m_station=0;
    ModuleInfo m_itemPCB;
    WorkStep  m_nextStep=FeedStep;
    FilmTearNode* m_filmTear = nullptr;
	PCBCleanNode* m_cleanNode = nullptr;
	DirtyNode* m_dirtyNode = nullptr;
};

#endif // TURNTABLEWORKSTEP_H
