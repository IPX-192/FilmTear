#include "ThreadClean.h"
#include "VisAppBus.h"
#include "VisMotorManager.h"
#include "VisAppThreadPool.h"
#include "ParamManager.h"
#include "CylinderCtrl.h"

using namespace  VisMotorToolSpace;

ThreadClean::ThreadClean(QObject *parent) : QObject(parent)
{
    VisAppBus::subscibeEvent(this,"PipeLineTrayReady");
}

int ThreadClean::BeginClean()
{
    ShowLog(Log_Clean, 0, Log_Info, QString(u8"开始清洗PCB"));
	int nRes = GrabPCB();
	if (nRes != 0)return nRes;
	nRes = CleanPCB();
	if (nRes != 0)return nRes;
	nRes = PlacePCB();
	if (nRes != 0)return nRes;
    ShowLog(Log_Clean, 0, Log_Info, QString(u8"清洗PCB结束"));
    sigBlankPipeLineTray(m_funcType, m_trayInfo);
    return 0;
}

int ThreadClean::GrabPCB()
{
	//夹爪上升
	int nRes = CylinderCtrl::instance()->event_SetCleanPCBGripUp(0, true);
	if (nRes != 0)return nRes;
	//张开夹爪
	nRes = SetGripClose(false);
	if (nRes != 0)return nRes;
	//到抓取位
	nRes = VisMotorInstance->MovePositionAbs(PipeLineCleanPCB_Grab);
	if (nRes != 0)return nRes;
	//夹爪下降
	nRes = CylinderCtrl::instance()->event_SetCleanPCBGripUp(0, false);
	if (nRes != 0)return nRes;
	//关闭夹爪
	nRes = SetGripClose(true);
	if (nRes != 0)return nRes;
	//夹爪上升
	nRes = CylinderCtrl::instance()->event_SetCleanPCBGripUp(0, true);
	if (nRes != 0)return nRes;
	//检查模组
	bool exist = false;
	nRes = CheckGripModuleExist(exist);
	if (nRes != 0)return nRes;
	if (!exist) {
		QString errInfo = QString(u8"流线清洗夹爪夹取后未检测到PCB");
		ShowLog(Log_Clean, 0, Log_Error, errInfo);
		QStringList listBtn;
		listBtn << u8"重新夹取" << u8"停止生产";
		nRes = VisAppBus::sendEvent("PopupUserMsgBox", listBtn, errInfo);
		if (nRes == 0) {
			nRes = GrabPCB();
			if (nRes != 0)return nRes;
		}
		else {
			VisAppBus::sendEvent("AutoEmg");
			return HardWareErr;
		}
	}
	return 0;
}

int ThreadClean::CleanPCB()
{
	//到抓取位
	int nRes = VisMotorInstance->MovePositionAbs(PipeLineCleanPCB_Clean);
	if (nRes != 0)return nRes;
	//清洗指令


	return 0;
}

int ThreadClean::PlacePCB()
{
	//到抓取位
	int nRes = VisMotorInstance->MovePositionAbs(PipeLineCleanPCB_Grab);
	if (nRes != 0)return nRes;
	//夹爪下降
	nRes = CylinderCtrl::instance()->event_SetCleanPCBGripUp(0, false);
	if (nRes != 0)return nRes;
	//张开夹爪
	nRes = SetGripClose(false);
	if (nRes != 0)return nRes;
	//夹爪上升
	nRes = CylinderCtrl::instance()->event_SetCleanPCBGripUp(0, true);
	if (nRes != 0)return nRes;
	return 0;
}

int ThreadClean::SetGripClose(bool close)
{
	int nRes = 0;
	if (close) {
		nRes = VisMotorInstance->MotorMoveAbs(MotorCleanGripX, 10);
		if (nRes != 0) return nRes;
		std::this_thread::sleep_for(std::chrono::milliseconds(200));
	}
	else {
		nRes = VisMotorInstance->MovePositionAbs(PipeLinePCBCleanGripOpen);
		if (nRes != 0) return nRes;
	}
    return 0;
}

int ThreadClean::CheckGripModuleExist(bool& exist)
{
	if (GlobalParam->flagOffline || GlobalParam->emptyRun) {
		exist = true;
		return 0;
	}

	curSportState state;
	if (!VisMotorInstance->ReadCurSportState((m_funcType == FeedHolder) ? MotorHolderGripX : MotorPCBGripX, state)) {
		QString errInfo = QString(u8"获取流线清洗夹爪夹取状态失败");
        ShowLog(Log_Clean, 0, Log_Error, errInfo);
		VisAppBus::sendEvent("PopupErrInfo", errInfo);
        return HardWareErr;
	}
	exist = (state == SPAORTSTOPANDNOCATCH);

	return 0;
}

void ThreadClean::sigBlankPipeLineTray(TrayFunc type , TrayInfo info)
{
    VisAppBus::sendEventDirect("BlankPipeLineTray",type, info);
}

int ThreadClean::event_PipeLineTrayReady(TrayFunc type, TrayInfo info)
{
	if (m_funcType != type)return 0;
	//空PCB直接退盘
	if (info.pcbBarCode.isEmpty()) {
		sigBlankPipeLineTray(type, info);
		return 0;
	}
	m_trayInfo = info;
    GlobalThreadPool->Commit(std::bind(&ThreadClean::BeginClean,this));
    return 0;
}
