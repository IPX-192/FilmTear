#include "TurntableWorkStep.h"
#include "VisAppBus.h"
#include "VisMotorManager.h"
#include "VisAppThreadPool.h"
#include "ParamManager.h"
#include "WorkNode/FilmTearNode.h"
#include "WorkNode/PCBCleanNode.h"
#include "WorkNode/DirtyNode.h"

using namespace  VisMotorToolSpace;

TurntableWorkStep::TurntableWorkStep(ModuleInfo item) :
    m_itemPCB(item)
{
    m_station=m_itemPCB.station;
	m_filmTear = new FilmTearNode(&m_itemPCB,m_station);
	m_cleanNode = new PCBCleanNode(&m_itemPCB,m_station);
	m_dirtyNode = new DirtyNode(&m_itemPCB,m_station);
}

TurntableWorkStep &TurntableWorkStep::operator =(const TurntableWorkStep &other)
{
    this->m_itemPCB=other.m_itemPCB;
    this->m_nextStep =other.m_nextStep;
    return *this;
}

//PCB 刚放到转盘治具时执行，初始化工序指针，第一道工序设为撕膜FilmTearStep。
int TurntableWorkStep::PreWork()
{
    ShowLog(Log_Fixture, m_station, Log_Info, QString(u8"开始预处理动作"));
    m_nextStep =FilmTearStep;
    return 0;
}

int TurntableWorkStep::Process()
{
    int nRes=0;
    if (m_nextStep == FilmTearStep)
        nRes = m_filmTear->Process();
    else if (m_nextStep == CleanStep)
        nRes = m_cleanNode->Process();
    else if (m_nextStep == DirtyStep)
        nRes = m_dirtyNode->Process();
    else if (m_nextStep == BlankStep)
        nRes = ProcessBlank();
    m_nextStep = (WorkStep)(m_nextStep + 1);

    return nRes;
}

ModuleInfo TurntableWorkStep::GetModuleInfo()
{
    return m_itemPCB;
}

//空工序，代表执行结束
int TurntableWorkStep::ProcessBlank()
{
    ShowLog(Log_Fixture, m_station, Log_Info, QString(u8"开始下料前动作%1").arg(m_station + 1));
    ShowLog(Log_Fixture, m_station, Log_Info, QString(u8"下料前动作结束%1").arg(m_station + 1));
    return 0;
}


