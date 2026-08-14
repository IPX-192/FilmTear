#include "CylinderCtrl.h"
#include "VisAppBus.h"
#include "VisMotorManager.h"
#include "VisMotorToolData.h"
#include "ParamManager.h"

using namespace  VisMotorToolSpace;

SINGLETON_IMPL(CylinderCtrl)
CylinderCtrl::CylinderCtrl(QObject *parent) : QObject(parent)
{
    VisAppBus::subscibeEvent(this, "SetBlockUp");
    VisAppBus::subscibeEvent(this, "CheckSkew");
    VisAppBus::subscibeEvent(this,"SetPushUp");
    VisAppBus::subscibeEvent(this, "SetGripClose");
    VisAppBus::subscibeEvent(this, "SetCleanPCBGripUp");
    VisAppBus::subscibeEvent(this, "SetTurntableGripUp");
}

int CylinderCtrl::CheckIoState(QString ioName, int level, QString errInfo)
{
    if (GlobalParam->flagOffline)return 0;
    if(VisMotorInstance->GetIoInput(ioName) != level)
    {
        ShowSystemLog(Log_Error, errInfo);
        QStringList iocheck;
        QVector<int> normalLevel;
        iocheck << ioName;
        normalLevel << level;
        int nRet = VisAppBus::sendEvent("PopupErrNotify", errInfo, iocheck, normalLevel);
        if (nRet != 0) return nRet;
    }
    return 0;
}

int CylinderCtrl::CheckIoState(QStringList ioName, int level, QString errInfo)
{
    if (GlobalParam->flagOffline)return 0;
    for (int i = 0; i < ioName.size(); i++) {
		if (VisMotorInstance->GetIoInput(ioName.at(i)) != level){
			ShowSystemLog(Log_Error, errInfo);
			QStringList iocheck;
			QVector<int> normalLevel;
			iocheck << ioName;
			for (int j = 0; j < ioName.size(); j++)
				normalLevel << level;
			int nRet = VisAppBus::sendEvent("PopupErrNotify", errInfo, iocheck, normalLevel);
			if (nRet != 0) return nRet;
		}
    }
	
	return 0;
}

int CylinderCtrl::SetDoubleIOOut(IOState out,IOState in1,IOState in2,QString errInfo)
{
    int nRet = VisMotorInstance->SetDoubleIoOutput(out.ioName, (IOLevel)out.level);
    if (nRet != 0) return nRet;
    if (!VisMotorInstance->SelectIoInput(in1.ioName, (IOLevel)in1.level, in2.ioName, (IOLevel)in2.level, 5000)) {
        ShowSystemLog(Log_Error, errInfo);
        QStringList iocheck;
        QVector<int> normalLevel;
        iocheck << in1.ioName << in2.ioName;
        normalLevel << in1.level << in2.level;
        nRet = VisAppBus::sendEvent("PopupErrNotify", errInfo, iocheck, normalLevel);
    }
    return nRet;
}

int CylinderCtrl::SetSigIOOut(IOState out, IOState in1, IOState in2, QString errInfo)
{
    int nRet = VisMotorInstance->SetIoOutput(out.ioName, (IOLevel)out.level);
    if (nRet != 0) return nRet;
    if (!VisMotorInstance->SelectIoInput(in1.ioName, (IOLevel)in1.level, in2.ioName, (IOLevel)in2.level, 5000)) {
        ShowSystemLog(Log_Error, errInfo);
        QStringList iocheck;
        QVector<int> normalLevel;
		iocheck << in1.ioName << in2.ioName;
		normalLevel << in1.level << in2.level;
        nRet = VisAppBus::sendEvent("PopupErrNotify", errInfo, iocheck, normalLevel);
    }
    return nRet;
}

int CylinderCtrl::SetSigIOOut(IOState out, IOState in, QString errInfo)
{
	int nRet = VisMotorInstance->SetIoOutput(out.ioName, (IOLevel)out.level);
	if (nRet != 0) return nRet;
	if (!VisMotorInstance->SelectIoInput(in.ioName, (IOLevel)in.level, 5000)) {
		ShowSystemLog(Log_Error, errInfo);
		QStringList iocheck;
		QVector<int> normalLevel;
		iocheck << in.ioName;
		normalLevel << in.level;
		nRet = VisAppBus::sendEvent("PopupErrNotify", errInfo, iocheck, normalLevel);
	}
	return nRet;
}

int CylinderCtrl::SetDoubleIOOut(IOState out, IOState in, QString errInfo)
{
	int nRet = VisMotorInstance->SetDoubleIoOutput(out.ioName, (IOLevel)out.level);
	if (nRet != 0) return nRet;
	if (!VisMotorInstance->SelectIoInput(in.ioName, (IOLevel)in.level, 5000)) {
		ShowSystemLog(Log_Error, errInfo);
		QStringList iocheck;
		QVector<int> normalLevel;
		iocheck << in.ioName;
		normalLevel << in.level;
		nRet = VisAppBus::sendEvent("PopupErrNotify", errInfo, iocheck, normalLevel);
	}
	return nRet;
}


int CylinderCtrl::event_SetBlockUp(TrayFunc type, bool up)
{
   /* if(up){
        QString errInfo = QString(u8"流水线%1阻挡气缸上升时,检测到托盘信号").arg(index ? u8"下料" : u8"上料");
        int nRes = CheckIoState(index ? IN_PipeLineBlankBlock : IN_PipeLineFeedBlock,IO_OFF,errInfo);
        if (nRes != 0) return nRes;
    }*/
    QMap<TrayFunc, QString>trayBlockName;
    trayBlockName[EmptyBuf] = u8"回流空盘缓存";
    trayBlockName[FeedPCB] = u8"上料PCB";
    trayBlockName[PCBClean] = u8"PCB清洗";
    trayBlockName[FeedTurntable] = u8"上料转盘";
    if (!trayBlockName.contains(type))return 0;

    QMap<TrayFunc, QString>outIOName;
    outIOName[EmptyBuf] = Out_PipeLineBackFlowBlockUp;
    outIOName[FeedPCB] = Out_PipeLineFeedPCBBlockUp;
    outIOName[PCBClean] = Out_PipeLinePCBCleanBlockUp;
    outIOName[FeedTurntable] = Out_PipeLineTurntableBlockUp;

    QMap<TrayFunc, QStringList>inIOName;
    inIOName[EmptyBuf]  << IN_PipeLineBackFlowBlockDown;
    inIOName[FeedPCB]  << IN_PipeLineFeedPCBBlockDown;
    inIOName[PCBClean]  << IN_PipeLinePCBCleanBlockDown;
    inIOName[FeedTurntable]  << IN_PipeLineTurntableBlockDown;

	IOState out, in;
    out.ioName= outIOName[type];
    out.level=up ? IO_ON : IO_OFF;
    in.ioName= inIOName[type].at(0);
    in.level=up ? IO_OFF : IO_ON;
  
    QString errInfo = QString(u8"5秒内%1阻挡气缸没有检测到在%2信号").arg(trayBlockName[type]).arg(up ? u8"上" : u8"下");
    int nRes = SetSigIOOut(out,in,errInfo);
    return nRes;
}

int CylinderCtrl::event_SetPushUp(TrayFunc type, bool up)
{
	QMap<TrayFunc, QString>trayBlockName;
    trayBlockName[FeedPCB] = u8"上料PCB";
    trayBlockName[PCBClean] = u8"PCB清洗";
    trayBlockName[FeedTurntable] = u8"上料转盘";
    if (!trayBlockName.contains(type))return 0;

	QMap<TrayFunc, QString>outIOName;
    outIOName[FeedPCB] = Out_PipeLineFeedPCBPushUp;
    outIOName[PCBClean] = Out_PipeLineCleanPCBPushUp;
    outIOName[FeedTurntable] = Out_PipeLineTurntablePushUp;

	QMap<TrayFunc, QStringList>inIOName;
    inIOName[FeedPCB]  << IN_PipeLineFeedPCBPushDown;
    inIOName[PCBClean]  << IN_PipeLineCleanPCBPushDown;
    inIOName[FeedTurntable]  << IN_PipeLineTurntableFeedPushDown;

	IOState out, in;
	out.ioName = outIOName[type];
	out.level = up ? IO_ON : IO_OFF;
	in.ioName = inIOName[type].at(0);
	in.level = up ? IO_OFF : IO_ON;
    QString errInfo;
    if(up)
        errInfo = QString(u8"5秒内%1顶升没有检测到在上信号").arg(trayBlockName[type]);
    else
        errInfo = QString(u8"5秒内%1顶升没有检测到在下信号").arg(trayBlockName[type]);
	int nRes = SetDoubleIOOut(out, in, errInfo);
	if (nRes != 0)return nRes;
    if(up)
		std::this_thread::sleep_for(std::chrono::milliseconds(3000));
    return nRes;
}

int CylinderCtrl::event_CheckSkew(TrayFunc type)
{
	QMap<TrayFunc, QString>trayBlockName;
	trayBlockName[FeedPCB] = u8"上料PCB";
	trayBlockName[PCBClean] = u8"PCB清洗";
	trayBlockName[FeedTurntable] = u8"上料转盘";

	QMap<TrayFunc, QStringList>inIOName;
	inIOName[FeedPCB] << IN_PipeLineFeedPCBTilt1 << IN_PipeLineFeedPCBTilt2;
	inIOName[PCBClean] << IN_PipeLineCleanPCBTilt1 << IN_PipeLineCleanPCBTilt2;
	inIOName[FeedTurntable] << IN_PipeLineTurntableFeedTilt1 << IN_PipeLineTurntableFeedTilt2;

    IOLevel level = IO_ON;
    QString errInfo = QString(u8"%1顶托盘时，检测到歪斜信号").arg(trayBlockName[type]);
	
	return  CheckIoState(inIOName[type], level, errInfo);
}

int CylinderCtrl::event_SetGripClose1(TrayFunc type, bool close)
{
	/*if (close) {
		return VisMotorInstance->MovePositionAbs(close ? FeedGripLoosen : FeedGripFix);
	}
	else {
		return VisMotorInstance->MovePositionAbs(open ? BlankGripLoosen : BlankGripFix);
	}*/

    return 0;
}

int CylinderCtrl::event_SetCleanPCBGripUp(int index, bool up)
{
	IOState out, in1, in2;
	out.ioName = index? Out_TurntableCleanGripUp: Out_PipeLineCleanGripUp;
	out.level = up ? IO_ON : IO_OFF;
    in1.ioName = index ? IN_TurntableCleanGripUp : IN_PipeLineCleanGripUp;
	in1.level = up ? IO_ON : IO_OFF;
	in2.ioName = index ? IN_TurntableCleanGripDown : IN_PipeLineCleanGripDown;
	in2.level = up ? IO_OFF : IO_ON;
    QString errInfo = QString(u8"5秒内%1夹爪没有检测到在%2信号").arg(index ? u8"转盘清洗PCB" : u8"流线清洗PCB").arg(up ? u8"上" : u8"下");
	int nRes = SetDoubleIOOut(out, in1, in2, errInfo);
	return nRes;
}

int CylinderCtrl::event_SetTurntableGripUp(int index, bool up)
{
	IOState out, in1, in2;
	out.ioName = index ? Out_PipeLineBlankTurntableGripUp : Out_PipeLineFeedTurntableGripUp;
	out.level = up ? IO_ON : IO_OFF;
	in1.ioName = index ? IN_PipeLineBlankTurntableGripUp : IN_PipeLineFeedTurntableGripUp;
	in1.level = up ? IO_ON : IO_OFF;
	in2.ioName = index ? IN_PipeLineBlankTurntableGripDown : IN_PipeLineFeedTurntableGripDown;
	in2.level = up ? IO_OFF : IO_ON;
	QString errInfo = QString(u8"5秒内转盘%1夹爪没有检测到在%2信号").arg(index ? u8"下料" : u8"上料").arg(up ? u8"上" : u8"下");
	int nRes = SetDoubleIOOut(out, in1, in2, errInfo);
	return nRes;
}
