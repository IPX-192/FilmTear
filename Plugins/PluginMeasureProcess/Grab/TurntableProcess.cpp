#include "TurntableProcess.h"
#include "VisAppBus.h"
#include "VisMotorManager.h"
#include "VisAppThreadPool.h"
#include "ParamManager.h"
#include "VisMotorToolData.h"

using namespace  VisMotorToolSpace;

TurntableProcess::TurntableProcess(QObject *parent) : QObject(parent)
{
    VisAppBus::subscibeEvent(this, "PCBReady");
    VisAppBus::subscibeEvent(this, "GrabTurntablePCBEnd");
    VisAppBus::subscibeEvent(this, "SetClearState");
    VisAppBus::subscibeEvent(this, "GetCurFixtureIsIdle");
    VisAppBus::subscibeEvent(this, "ManualSelectFixture");
}

TurntableProcess::~TurntableProcess()
{
    stop();
}

int TurntableProcess::InitParam()
{
    m_listPCB.clear();
    m_clearFlag=false;
    m_blankGrabEnd=true;
    m_newPcbReady=false;
    for (int i = 0; i < 4; i++) {
        if (!GlobalParam->systemParam.shieldParam.turntable[i]) {
            m_curFixture = i;
            break;
        }
    }
    m_fixtureIdle=true;
    GlobalThreadPool->Clear();
    ShowLog(Log_Fixture, m_curFixture, Log_Debug, QString(u8"等待放置新产品"));
    return 0;
}

bool TurntableProcess::doTask()
{
    if(VisMotorInstance->IsEmgStop())return false;
    int nRes=0;
    QVector<TurntableWorkStep*>listPCB;
    for(int i=0;i<m_listPCB.size();i++){
        listPCB<<m_listPCB[i];
    }
    //判断最新一个是否需要预处理
    if(listPCB.back()->IsStepBegin()){
        nRes= listPCB.back()->PreWork();
        if(nRes!=0)return false;
    }
    //旋转转盘
    nRes=RotateTurntable();
    if(nRes!=0)return false;
    //先提交任务到线程池
    QStringList listTask;
    for(int i=0;i<listPCB.size();i++){
        listTask<<QString("TurntableWorkStep_%1").arg(i);
        GlobalThreadPool->Commit_Topic(listTask.at(i),std::bind(&TurntableWorkStep::Process,listPCB[i]));
    }
    ModuleInfo& firstModule = listPCB.first()->GetModuleInfo();
    //当前上料位治具没屏蔽,且不需下料抓取
    bool turntableShiled=GlobalParam->systemParam.shieldParam.turntable[m_curFixture];
    if((!turntableShiled)&&(firstModule.station!=m_curFixture)){
        m_blankGrabEnd=true;
        nRes=WaitBlankAndNewPCB();
        if(nRes!=0)return false;
    }

    //等待任务完成
    for(int i=0;i<listTask.size();i++){
        nRes=GlobalThreadPool->WaitTask(listTask.at(i));
        if (nRes != 0)return false;
        //首任务判断是否最后工序
        if (i == 0 && firstModule.station == m_curFixture) {
            sigPCBTestEnd(firstModule);
            nRes = WaitBlankAndNewPCB();
            if (nRes != 0)return false;
        }
    }
	//若m_listPCB存储不是指针类型,需在此处移除,sigPCBTestEnd移除有bug,
	// 暂不清楚原因，大概率是TurntableWorkStep在多线程中访问中,移除了单个元素,导致的异常
	/*if (m_listPCB.size() > 4)
		m_listPCB.pop_front();*/

    return true;
}

int TurntableProcess::RotateTurntable()
{
    for (int i = 0; i < 4; i++)
        ShowLog(Log_Fixture, i, Log_Debug, QString(u8"开始旋转转盘"));
    //旋转
    double initAngle= VisMotorDataInstance->GetPosMap(TurntableInitPos)[MotorTurntableR];
    m_curFixture++;
    if(m_curFixture>=4)
        m_curFixture=0;
    double  rotateAngle=m_curFixture*90+initAngle;
    int nRes=VisMotorInstance->MotorMoveAbs(MotorTurntableR,rotateAngle);
    if(nRes!=0)return nRes;

    return 0;
}

int TurntableProcess::WaitBlankAndNewPCB()
{
    m_newPcbReady=false;
    m_fixtureIdle=true;
    ShowLog(Log_Fixture, m_curFixture, Log_Debug, QString(u8"等待放置新产品"));
    //等待上新料
    while(1){
        if(VisMotorInstance->IsEmgStop())return HardWareErr;
        //进入清料状态并且下料抓取结束
        if(m_clearFlag && m_blankGrabEnd){
            ShowLog(Log_Fixture, m_curFixture, Log_Debug, QString(u8"清料状态下料完毕"));
            if (!m_listPCB.size()) {
                for (int i = 0; i < 4; i++)
                    ShowLog(Log_Fixture, i, Log_Debug, QString(u8"退出清料状态"));
                m_clearFlag = false;
            }
               
            return 0;
        }
        //新料就绪
        if(m_newPcbReady){
            m_blankGrabEnd=false;
            int nRes=m_listPCB.back()->PreWork();
            return nRes;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    return 0;
}

void TurntableProcess::sigPCBTestEnd(ModuleInfo item)
{
    m_blankGrabEnd=false;
	delete m_listPCB.front();
	m_listPCB.pop_front();
    VisAppBus::sendEventDirect("PCBTestEnd",item);
}

int TurntableProcess::event_SetClearState()
{
    m_clearFlag=true;
    return 0;
}

int TurntableProcess::event_PCBReady(ModuleInfo item)
{
    if(VisMotorInstance->IsEmgStop())return HardWareErr;
    ShowLog(Log_Fixture, m_curFixture, Log_Debug, QString(u8"新产品就绪"));
    m_fixtureIdle=false;
    item.station = m_curFixture;
   // TurntableWorkStep pcbItem(item);
    TurntableWorkStep* pcbItem = new TurntableWorkStep(item);
    m_listPCB.push_back(pcbItem);
    m_newPcbReady=true;
    start();

    return 0;
}

int TurntableProcess::event_GrabTurntablePCBEnd()
{
    m_blankGrabEnd=true;
    return 0;
}

int TurntableProcess::event_GetCurFixtureIsIdle(bool &idle,int&indexFixture)
{
    idle=m_fixtureIdle;
    return 0;
}

int TurntableProcess::event_ManualSelectFixture(int jigNo)
{
    return 0;
}

