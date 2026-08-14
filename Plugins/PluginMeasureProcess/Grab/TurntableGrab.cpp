#include "TurntableGrab.h"
#include "VisAppBus.h"
#include "VisMotorManager.h"
#include "VisAppThreadPool.h"
#include "ParamManager.h"
#include "TurntableProcess.h"
#include "VisMotorToolData.h"
#include "CylinderCtrl.h"

using namespace  VisMotorToolSpace;

TurntableGrab::TurntableGrab(QObject *parent) : QObject(parent)
{
    m_turntableProcess=new TurntableProcess(this);
    VisAppBus::subscibeEvent(this, "PipeLineTrayReady");
    VisAppBus::subscibeEvent(this, "PCBTestEnd");
}

TurntableGrab::~TurntableGrab()
{
	delete m_turntableProcess;
    stop();
}

void TurntableGrab::InitParam()
{
    m_errInfo="";
    m_trayFeedNum=0;
    m_waitFeedTray=true;
    m_vecFeedPCB.clear();
    m_listPCBBlank.clear();
    m_vecBlankPCB.clear();
    m_clearFlag = false;
    m_turntableProcess->InitParam();
    start();
}

bool TurntableGrab::doTask()
{
    if (VisMotorInstance->IsEmgStop())return false;
    if (!Process()) {
        if (!m_errInfo.isEmpty())
            ShowSystemLog(Log_Error, m_errInfo);
        return false;
    }
    return true;
}

bool TurntableGrab::Process()
{
    if (!m_clearFlag) {
		//等待托盘
		if (m_waitFeedTray) {
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			return true;
		}
		//从料盘抓料
		int nRes = GrabTrayModule();
		if (nRes != 0) return false;
		if (!m_vecFeedPCB.size()) return true;
		//等待治具空闲并下料抓取
		nRes = WaitFixtureIdle();
		if (nRes != 0)return false;
		//放置PCB到测试位
		nRes = PlaceTurntable();
		if (nRes != 0)return false;
		//PCB下料到托盘
		if (m_vecBlankPCB.size()) {
			nRes = BlankPCBToTray();
			if (nRes != 0)return false;
		}
    }
	else {
		//等待治具空闲并下料抓取
		int nRes = WaitFixtureIdle();
		if (nRes != 0)return false;
		//PCB下料到托盘
		if (m_vecBlankPCB.size()) {
			nRes = BlankPCBToTray();
			if (nRes != 0)return false;
		}
		if (m_numTotalGrab == 0) {
			m_clearFlag = false;
			ShowLog(Log_TurntableGrab, 0, Log_Debug, QString(u8"剩余模组下料完毕"));
		}
	}

    return true;
}

int TurntableGrab::GrabTrayModule()
{
	m_vecFeedPCB.clear();
    //开始抓料
    ShowLog(Log_TurntableGrab, 0, Log_Info, QString(u8"开始抓取载具PCB"));
    //张开夹爪
	int nRes = SetGripClose(0, false);
	if (nRes != 0)return nRes;
	//到抓取位
	QMap<QString, double> mapGroup = VisMotorDataInstance->GetPosMap(TurntablePipeLineGrab);
	QMap<QString, double> moveGroup = mapGroup;
	moveGroup.remove(MotorTurntableGantryZ);
	nRes = VisMotorInstance->MoveAbsGroup(moveGroup);
	if (nRes != 0)return nRes;
	nRes = CylinderCtrl::instance()->event_SetTurntableGripUp(0, false);
	if (nRes != 0)return nRes;
	nRes = VisMotorInstance->MotorMoveAbs(MotorTurntableGantryZ, mapGroup[MotorTurntableGantryZ]);
	if (nRes != 0)return nRes;
    //闭合夹爪
    nRes = SetGripClose(0, true);
    if (nRes != 0)return nRes;
	//移动到安全位
	nRes = VisMotorInstance->MovePositionAbs(TurntableFeedGripSafe);
	if (nRes != 0)return nRes;
	nRes = CylinderCtrl::instance()->event_SetTurntableGripUp(0, true);
	if (nRes != 0)return nRes;
	bool existPCB = false;
    nRes = CheckGripModuleExist(0, existPCB);
    if (nRes != 0)return nRes;
 //   //如果没抓到
	//if (!m_existPCB) {
 //       ShowLog(Log_TurntableGrab, 0, Log_Debug, QString(u8"未抓取到载具PCB"));
	//	sigBlankPipeLineTray(m_funcType);
	//	return 0;
	//}

    //前4个托盘抓完PCB直接送走
    m_trayFeedNum++;
	m_curTrayInfo.pcbBarCode = "";
    if(m_trayFeedNum<5){
		m_curTrayInfo.empty = true;
        sigBlankPipeLineTray(m_funcType,m_curTrayInfo);
    }
	ModuleInfo pcbInfo;
	pcbInfo.pcbCode = m_curTrayInfo.pcbBarCode;
	m_vecFeedPCB.push_back(pcbInfo);

    return 0;
}

int TurntableGrab::SetGripClose(int index,bool close)
{
	int nRes = 0;
	if (close) {
        //闭合夹爪:固定到0(张开按伺服配方)
        nRes = VisMotorInstance->MotorMoveAbs(index ? MotorTurntableBlankGripX : MotorTurntableFeedGripX, 0);
		if (nRes != 0) return nRes;
		std::this_thread::sleep_for(std::chrono::milliseconds(200));
	}
	else {
        nRes = VisMotorInstance->MovePositionAbs(index ? TurntableBlankGripOpen : TurntableFeedGripOpen);
		if (nRes != 0) return nRes;
	}
	return 0;
}

int TurntableGrab::CheckGripModuleExist(int index,bool& exist)
{
	if (GlobalParam->flagOffline || GlobalParam->emptyRun) {
		exist = true;
		return 0;
	}

	curSportState state;
	if (!VisMotorInstance->ReadCurSportState((m_funcType == FeedHolder) ? MotorHolderGripX : MotorPCBGripX, state)) {
		m_errInfo = QString(u8"获取转盘%1夹爪夹取状态失败").arg(index ? u8"下料" : u8"上料");
		ShowSystemLog(Log_Error, m_errInfo);
		VisAppBus::sendEvent("PopupErrInfo", m_errInfo);
		return HardWareErr;
		return -1;
	}
	exist = (state == SPAORTSTOPANDNOCATCH);

	return 0;
}

//等待转盘任意治具空闲
int TurntableGrab::WaitFixtureIdle()
{
    ShowLog(Log_TurntableGrab, 0, Log_Info, QString(u8"等待治具空闲"));
    m_vecBlankPCB.clear();
    int nRes=0;
    while(1){
        if(VisMotorInstance->IsEmgStop())return HardWareErr;
        //治具空闲
		bool idle = false;
        sigGetCurFixtureIsIdle(idle, m_curIdleFixture);
        if(!idle){
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            continue;
        }
        m_mutexBlankPCB.lock();
        //无下料模组
        if(!m_listPCBBlank.size()){
            m_mutexBlankPCB.unlock();
            ShowLog(Log_TurntableGrab, 0, Log_Info, QString(u8"无待下料模组"));
            //检查治具是否有料

            return 0;
        }
        ModuleInfo module=m_listPCBBlank.front();
        m_listPCBBlank.pop_front();
        m_mutexBlankPCB.unlock();
        m_vecBlankPCB.push_back(module);
        nRes=GrabTurntablePCB();
        break;
    }

    return nRes;
}

int TurntableGrab::GrabTurntablePCB()
{
    ModuleInfo module = m_vecBlankPCB.at(0);
    ShowLog(Log_TurntableGrab, 0, Log_Info, QString(u8"开始下料抓取治具%1模组").arg(module.station + 1));
	//张开夹爪
	int nRes = SetGripClose(1, false);
	if (nRes != 0)return nRes;
	//到抓取位
	nRes = VisMotorInstance->MovePositionAbs(TurntableBlankGrab);
	if (nRes != 0)return nRes;
	nRes = CylinderCtrl::instance()->event_SetTurntableGripUp(1, false);
	if (nRes != 0)return nRes;
	//闭合夹爪
	nRes = SetGripClose(1, true);
	if (nRes != 0)return nRes;
	//夹料后上升夹爪,避免移动时碰撞
	nRes = CylinderCtrl::instance()->event_SetTurntableGripUp(1, true);
	if (nRes != 0)return nRes;
	//移动到安全位
	nRes = VisMotorInstance->MovePositionAbs(TurntableFeedGripSafe);
	if (nRes != 0)return nRes;
    bool exist = false;
	nRes = CheckGripModuleExist(1, exist);
	if (nRes != 0)return nRes;
	//如果没抓到
	if (!exist) {
		ShowLog(Log_TurntableGrab, 0, Log_Error, QString(u8"未抓取到转盘PCB"));
		QStringList listBtn;
		listBtn << u8"重新夹取" << u8"停止生产";
		nRes = VisAppBus::sendEvent("PopupUserMsgBox", listBtn, QString(u8"未抓取到转盘PCB"));
		if (nRes == 0) {
			nRes = GrabTurntablePCB();
			if (nRes != 0)return nRes;
            return 0;
		}
		else {
			VisAppBus::sendEvent("AutoEmg");
			return HardWareErr;
		}
		return 0;
	}

    sigGrabTurntablePCBEnd();

    return 0;
}

//放置 PCB 到转盘治具
int TurntableGrab::PlaceTurntable()
{
	ShowLog(Log_TurntableGrab, 0, Log_Info, QString(u8"开始上料放置到转盘"));
	ModuleInfo module = m_vecFeedPCB.at(0);
	module.station = m_curIdleFixture;
	//到放置位
	int nRes = VisMotorInstance->MovePositionAbs(TurntableFeedPlace);
	if (nRes != 0)return nRes;
	nRes = CylinderCtrl::instance()->event_SetTurntableGripUp(0, false);
	if (nRes != 0)return nRes;

	//张开夹爪
	nRes = SetGripClose(0, false);
	if (nRes != 0)return nRes;
	//移动到安全位
	nRes = VisMotorInstance->MovePositionAbs(TurntableFeedGripSafe);
	if (nRes != 0)return nRes;
	bool exist = false;
	//检查模组
	nRes = CheckGripModuleExist(0, exist);
	if (nRes != 0)return nRes;
	if (GlobalParam->flagOffline || GlobalParam->emptyRun) {
		exist = false;
	}
	if (exist) {
		m_errInfo = QString(u8"转盘上料夹爪松开后仍检测到模组");
		ShowLog(Log_TurntableGrab, 0, Log_Error, m_errInfo);
		VisAppBus::sendEvent("PopupErrInfo", m_errInfo);
		return HardWareErr;
	}

    sigPCBReady(module);
	//到放置位XY
	QMap<QString, double> mapGroup = VisMotorDataInstance->GetPosMap(TurntablePipeLinePlace);
	QMap<QString, double> moveGroup = mapGroup;
	moveGroup.remove(MotorTurntableGantryZ);
	nRes = VisMotorInstance->MoveAbsGroup(moveGroup);
	if (nRes != 0)return nRes;
    return 0;
}

int TurntableGrab::BlankPCBToTray()
{
    ShowLog(Log_TurntableGrab, 0, Log_Info, QString(u8"开始下料放置到载具"));
	ModuleInfo module = m_vecBlankPCB.at(0);
	//到放置位
	int nRes = VisMotorInstance->MovePositionAbs(TurntablePipeLinePlace);
	if (nRes != 0)return nRes;
	//QMap<QString, double> mapGroup = VisMotorDataInstance->GetPosMap(TurntablePipeLinePlace);
	//QMap<QString, double> moveGroup = mapGroup;
	//moveGroup.remove(MotorTurntableGantryZ);
	//int nRes = VisMotorInstance->MoveAbsGroup(moveGroup);
	//if (nRes != 0)return nRes;
	nRes = CylinderCtrl::instance()->event_SetTurntableGripUp(1, false);
	if (nRes != 0)return nRes;
	/*nRes = VisMotorInstance->MotorMoveAbs(MotorTurntableGantryZ, mapGroup[MotorTurntableGantryZ]);
	if (nRes != 0)return nRes;*/


	//张开夹爪
	nRes = SetGripClose(1, false);
	if (nRes != 0)return nRes;
	//移动到安全位
	nRes = VisMotorInstance->MovePositionAbs(TurntableFeedGripSafe);
	if (nRes != 0)return nRes;
	nRes = CylinderCtrl::instance()->event_SetTurntableGripUp(1, true);
	if (nRes != 0)return nRes;
	bool exist = false;
    //检查模组
	nRes = CheckGripModuleExist(1, exist);
	if (nRes != 0)return nRes;
	if (GlobalParam->flagOffline || GlobalParam->emptyRun) {
		exist = false;
	}
	if (exist) {
		m_errInfo = QString(u8"转盘下料夹爪松开后仍检测到模组");
		ShowLog(Log_TurntableGrab, 0, Log_Error, m_errInfo);
		VisAppBus::sendEvent("PopupErrInfo", m_errInfo);
		return HardWareErr;
	}

    m_vecBlankPCB.clear();
	//工序NG或上游NG都保持result=false;工序OK不清除上游已设的NG
	if (!module.result) {
		m_curTrayInfo.result = false;
		m_curTrayInfo.errInfo = module.ngReason;
	}
	m_curTrayInfo.pcbBarCode = module.pcbCode;
	m_numTotalGrab--;
    sigBlankPipeLineTray(m_funcType,m_curTrayInfo);

    return 0;
}

void TurntableGrab::sigPCBReady(ModuleInfo info)
{
    VisAppBus::sendEventDirect("PCBReady",info);
}

void TurntableGrab::sigGrabTurntablePCBEnd()
{
    VisAppBus::sendEventDirect("GrabTurntablePCBEnd");
}

void TurntableGrab::sigBlankPipeLineTray(TrayFunc type , TrayInfo info)
{
    m_waitFeedTray = true;
	if(info.pcbBarCode.isEmpty())
		ShowLog(Log_TurntableGrab, 0, Log_Info, QString(u8"下料空载具,载具码:%1,").arg(info.trayCode));
	else
	    ShowLog(Log_TurntableGrab, 0, Log_Info, QString(u8"下料绑定载具码:%1,壳体码:%2,PCB码:%3")
		   .arg(info.trayCode).arg(info.holderBarCode).arg(info.pcbBarCode));
    VisAppBus::sendEventDirect("BlankPipeLineTray",type, info);
}

void TurntableGrab::sigGetCurFixtureIsIdle(bool& idle, int& indexFixture)
{
    VisAppBus::sendEventDirect("GetCurFixtureIsIdle", idle, indexFixture);
}

int TurntableGrab::event_PipeLineTrayReady(TrayFunc type, TrayInfo info)
{
    if (m_funcType != type)return 0;
	//if (!isRun()) {
	//	double initAngle = VisMotorDataInstance->GetPosMap(TurntableInitPos)[MotorTurntableR]; //转盘
	//	int curFixture = 0;
	//	for (int i = 0; i < 4; i++) {
	//		if (!GlobalParam->systemParam.shieldParam.turntable[i]) {
	//			curFixture = i;
	//			break;
	//		}
	//	}
	//	double  rotateAngle = curFixture * 90 + initAngle;
	//	int nRes=VisMotorInstance->MotorMoveAbs(MotorTurntableR, rotateAngle);
	//	if (nRes != 0)return -1;
	//	InitParam();
	//}
    //空盘直接退盘
	if (info.empty) {
		sigBlankPipeLineTray(type, info);
		return 0;
	}
	m_curTrayInfo = info;
    //无壳体有PCB，正常作业，初始状态前面4个盘只有PCB
	if (info.holderBarCode.isEmpty()) {
		m_numTotalGrab++;
		m_waitFeedTray = false;
	}
    else { 
        //有壳体无PCB,进入清料状态
        if (info.pcbBarCode.isEmpty()) {
			m_clearFlag = true;
			ShowLog(Log_TurntableGrab, 0, Log_Debug, QString(u8"开始清料"));
        }
        else {   //有壳体有PCB,正常作业
			m_numTotalGrab++;
			m_waitFeedTray = false;
        }
    }
   
    return 0;
}

int TurntableGrab::event_PCBTestEnd(ModuleInfo module)
{
    m_mutexBlankPCB.lock();
    m_listPCBBlank.push_back(module);
    m_mutexBlankPCB.unlock();
    return 0;
}
