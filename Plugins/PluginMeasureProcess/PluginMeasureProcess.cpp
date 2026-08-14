#include "PluginMeasureProcess.h"
#include <QMessageBox>
#include <QApplication>
#include <QTime>
#include <QTimer>
#include "VisMotorToolData.h"
#include "VisSignalsManager.h"
#include "VisMotorTool.h"
#include "VisAppBus.h"
#include "ParamManager.h"
#include "PipeLine/PipeLineManager.h"
#include "Grab/ThreadFeed.h"
#include "Grab/ThreadClean.h"
#include "Grab/TurntableGrab.h"
#include "CylinderCtrl.h"
#include "Grab/WorkNode/DirtyNode.h"
#include "../Common/NonBlockingMsgBox.h"

using namespace VisMotorToolSpace;


PluginMeasureProcess::PluginMeasureProcess()
{
    pluginID = "PluginMeasureProcess";
	pluginVersion = "1.0.1";
	pluginAuther = "wangwei";
	pluginAuthority = OPERATOR;
    CylinderCtrl::instance();
    VisAppBus::subscibeEvent(this, "ManualFeeding");
    VisAppBus::subscibeEvent(this, "TrayBlankEnd");
    VisAppBus::subscibeEvent(this, "ConnectGrip");
    VisAppBus::subscibeEvent(this, "SetProductEnd");
    VisAppBus::subscibeEvent(this, "GetProductEnd");
}

PluginMeasureProcess::~PluginMeasureProcess()
{
    //delete m_pipeLineManager;
}

int PluginMeasureProcess::OnInitialized()
{
    m_threadFeed[0] = new ThreadFeed(FeedPCB,this);
    m_threadFeed[1] = new ThreadFeed(FeedHolder,this);
    m_threadClean = new ThreadClean(this);
    m_turntableGrab = new TurntableGrab(this);
    m_pipeLineManager=new PipeLineManager(this);
	InitSlot();

	return 0;
}

void PluginMeasureProcess::InitActionList(Plugin_Interface* plugin)
{
	/*PluginActionInfo* action1 = new PluginActionInfo();
	action1->_actionName = "ReConnetPreDev";
	action1->_actionDetail = tr("重连上游");
	action1->_pAction = (FPTR_ACTION)(&PluginMeasureProcess::ReConnetPreDev);
	plugin->listAction.append(action1);*/
}


int PluginMeasureProcess::event_InitPosParam()
{
    QString filename = GlobalParam->recipeMotor.filepath + GlobalParam->recipeMotor.curRecipe + ".xml";
    bool bRet = VisMotorToolIns->SetPointFile(filename);
    ShowSystemLog(bRet ? Log_Info : Log_Error, QString(u8"伺服配方文件加载%1！").arg(bRet ? u8"成功" : u8"失败"));
    if (!bRet)  return -1;

	return 0;
}

int PluginMeasureProcess::event_AutoStart()
{
    if (VisMotorInstance->GetIoInput(In_Emg) == IO_OFF && !GlobalParam->flagOffline) {
        QMessageBox::warning(nullptr, u8"警告", u8"急停按钮未复位，无法启动流程");
        return -1;
    }
    //if (!VisMotorInstance->IsResetted() && !VisMotorDataInstance->m_flagOffline) {
    //	ShowSystemLog(Log_Error, QString(u8"设备未复位,不能开始生产"));
    //	return -1;
    //}

    if (QMessageBox::question(nullptr, u8"询问", u8"是否启动自动模式?") == QMessageBox::No)
        return -1;
    int nRes= event_InitPosParam();
    if (nRes != 0) return nRes;

    nRes = CheckIO();
	if (nRes != 0) return nRes;
	nRes = CheckDevicePos();
	if (nRes != 0) return nRes;
	nRes = InitIO();
	if (nRes != 0) return nRes;
	nRes = InitDevicePos();
    if (nRes != 0) return nRes;
	//初始化脏污检测器
    DirtyNode::InitDetector();
	//初始化状态
    m_threadFeed[0]->InitParam();
    m_threadFeed[1]->InitParam();
    m_turntableGrab->InitParam();
    if(m_pipeLineManager->Init()!=0){
        ShowSystemLog(Log_Error, QString(u8"设备连线服务初始化失败,不能开始生产"));
        return -1;
    }
    //待料
    m_waitTray = true;
	VisMotorDataInstance->m_running = true;
    VisAppBus::sendEvent("StartUph");
	ShowSystemLog(Log_Info, QString(u8"设备已进入生产模式"));
    VisAppBus::sendEvent("InTrayTask", FeedHolder);
	VisAppBus::sendEvent("InTrayTask", FeedPCB);

	return 0;
}

int PluginMeasureProcess::event_ClearModule()
{
    ShowSystemLog(Log_Info, QString(u8"清料流程未实现，不允许使用!"));
    return -1;
    if (VisMotorInstance->IsEmgStop()){
        ShowSystemLog(Log_Info, QString(u8"急停状态,无法清料,请清除报警后重试"));
        return 0;
    }
    if (m_clearModule) {
        ShowSystemLog(Log_Info, QString(u8"正在清料,请勿重复执行"));
        return 0;
    }
    ShowSystemLog(Log_Info, QString(u8"开始清料"));
//    if (VisMotorInstance->GetIoInput(IN_NgCheck1) == IO_OFF ||
//            VisMotorInstance->GetIoInput(IN_NgCheck2) == IO_OFF) {
//        ShowSystemLog(Log_Error, QString(u8"检测到NG盘有料，无法自动清料"));
//        return -1;
//    }
    m_clearModule = true;
    //Z到安全位
    if(0 != DoClearModule())
    {
        ShowSystemLog(Log_Info, QString(u8"清料失败"));
    }
    else
    {
        ShowSystemLog(Log_Info, QString(u8"清料完成"));
    }
    m_clearModule = false;
    return 0;
}

int PluginMeasureProcess::event_AutoPause()
{
	VisMotorInstance->SetPause();
    VisMotorInstance->SetIoOutput(Out_RedLight, IO_OFF);
    VisMotorInstance->SetIoOutput(Out_GreenLight, IO_OFF);
    VisMotorInstance->SetIoOutput(Out_YellowLight, IO_ON);
    VisAppBus::sendEvent("DevException", true);
	return 0;
}

int PluginMeasureProcess::event_AutoResume()
{
    VisMotorInstance->SetIoOutput(Out_RedLight, IO_OFF);
    VisMotorInstance->SetIoOutput(Out_GreenLight, IO_ON);
    VisMotorInstance->SetIoOutput(Out_YellowLight, IO_OFF);
	VisMotorInstance->ResumeMove();
    VisAppBus::sendEvent("DevException", false);
	return 0;
}

int PluginMeasureProcess::event_AutoEmg()
{
    m_clearModule = false;
    if (m_isEmgFlag)
        return 0;
    m_isEmgFlag = true;
    VisMotorDataInstance->m_running = false;
    VisAppBus::sendEvent("DevException", true);
	VisMotorInstance->SetIoOutput(Out_Start1, IO_OFF);
    VisMotorInstance->SetIoOutput(Out_Start2, IO_OFF);
    VisMotorInstance->SetIoOutput(Out_Reset, IO_OFF);
    VisMotorInstance->SetIoOutput(Out_Buzzer, IO_ON);
    VisMotorInstance->SetIoOutput(Out_YellowLight, IO_OFF);
    VisMotorInstance->SetIoOutput(Out_RedLight, IO_ON);
    VisMotorInstance->SetIoOutput(Out_GreenLight, IO_OFF);
	VisMotorInstance->SetIoOutput(Out_PipeLineMotorR, IO_OFF);
	VisMotorInstance->SetIoOutput(Out_PipeLineMotorL, IO_OFF);
	VisMotorInstance->SetIoOutput(Out_PipeLineBackFlowMotorL, IO_OFF);
	VisMotorInstance->SetIoOutput(Out_PipeLineBackFlowMotorR, IO_OFF);
	VisMotorInstance->SetEmgStop(true);
	std::this_thread::sleep_for(std::chrono::milliseconds(50));
	VisMotorInstance->MotorStop();             //停止所有轴运动
	VisMotorInstance->ClearPauseAxisInfo();
	ShowSystemLog(Log_Error, QString(u8"设备急停"));

	return 0;
}

int PluginMeasureProcess::event_AutoReset()
{
	if (m_inReset) {
		ShowSystemLog(Log_Error, QString(u8"正在复位中，请勿重复操作"));
		return 0; //复位中，忽略
	}
	m_inReset = true;

	if (!VisMotorInstance->OpenMotorEnable()){
		ShowSystemLog(Log_Error, QString(u8"电机使能失败"));
		m_inReset = false;
		return -1;
	}
	//等待电机使能完毕
	if (!VisMotorInstance->WaitMotorEnable()){
		ShowSystemLog(Log_Error, QString(u8"电机使能超时"));
		m_inReset = false;
		return -1;
	}
	ShowSystemLog(Log_Info, QString(u8"电机使能成功"));
	VisMotorInstance->SetIoOutput(Out_PipeLineBackFlowMotorPower, IO_ON);
	VisMotorInstance->SetIoOutput(Out_PipeLineMotorPower, IO_ON);
    //缩回接驳台
    VisMotorInstance->SetDoubleIoOutput(Out_TransferBack, IO_ON, Out_TransferExend, IO_OFF);
    m_inReset = false;

//	//检查能否复位
//	if (ResetPreCheck() != 0) {
//		m_inReset = false;
//		return -1;
//	}
//	//上升PCB清洗夹爪
//	for (int i = 0; i < 2; i++) {
//		int nRes = CylinderCtrl::instance()->event_SetCleanPCBGripUp(i, true);
//		if (nRes != 0) {
//			m_inReset = false;
//			return nRes;
//		}
//		nRes = CylinderCtrl::instance()->event_SetTurntableGripUp(i, true);
//		if (nRes != 0) {
//			m_inReset = false;
//			return nRes;
//		}

//	}
	//轴复位
	if (!VisMotorInstance->MakeHomeAllAxis()){
		ShowSystemLog(Log_Error, QString(u8"轴复位失败"));
		m_inReset = false;
		return -1;
	}
    ShowSystemLog(Log_Info, QString(u8"轴复位成功"));
    InitDevicePos();
//    //扫码抢连接
//    VisAppBus::sendEvent("Connect", ScanHolderCode, GlobalParam->hardwareParam.trayCodeParam.sIP);
//    VisAppBus::sendEvent("Connect", ScanPCBCode, GlobalParam->hardwareParam.pcbCodeParam.sIP);

//    // RFID Modbus 连接
//    auto& rfidParam = GlobalParam->hardwareParam.trayRfidDebugParam;
//    VisAppBus::sendEvent("ConnectModbus", rfidParam.comPort, rfidParam.StrbaudRate.toInt());
	
//    VisMotorInstance->SetIoOutput(Out_YellowLight, IO_ON);
//    VisMotorInstance->SetIoOutput(Out_RedLight, IO_OFF);
//    VisMotorInstance->SetIoOutput(Out_GreenLight, IO_OFF);
//    VisMotorInstance->SetIoOutput(Out_Buzzer, IO_OFF);
//	VisMotorInstance->SetIoOutput(Out_Start1, IO_OFF);
//	VisMotorInstance->SetIoOutput(Out_Start2, IO_OFF);
//	for (int i = FeedPCB; i <= FeedTurntable; i++) {
//		int nRes = CylinderCtrl::instance()->event_SetPushUp((TrayFunc)i, false);
//		if (nRes != 0) return nRes;
//	}
//	return 0;
}

void PluginMeasureProcess::sigMachineStatus(MachineRunStatus status)
{
	VisAppBus::sendEvent("SetMachineStatus", status);
}

int PluginMeasureProcess::event_ClearWarning()
{
    m_isEmgFlag = false;
    m_inReset = false;
    m_productEnd = false;     //重新进入生产模式,清除结束生产标志
    VisMotorInstance->SetEmgStop(false);
	VisMotorInstance->SetHandStop(false);
	//三色灯
    VisMotorInstance->SetIoOutput(Out_Buzzer, IO_OFF);
	if (!VisMotorDataInstance->m_running){
        VisMotorInstance->SetIoOutput(Out_YellowLight, IO_ON);
        VisMotorInstance->SetIoOutput(Out_GreenLight, IO_OFF);
	}
	else{
        VisMotorInstance->SetIoOutput(Out_YellowLight, IO_OFF);
        VisMotorInstance->SetIoOutput(Out_GreenLight, IO_ON);
	}
    VisMotorInstance->SetIoOutput(Out_RedLight, IO_OFF);

	return 0;
}

int PluginMeasureProcess::event_PopupWarning(QString info)
{
    ShowSystemLog(Log_Error, info);
    VisMotorInstance->SetIoOutput(Out_Buzzer, IO_ON);
    MsgBoxManager::instance().showWarning(nullptr, QStringLiteral("提示"), info, NonBlockingMsgBox::Ok);
    VisMotorInstance->SetIoOutput(Out_Buzzer, IO_OFF);

	return 0;
}

void PluginMeasureProcess::WaitTime(int ms)
{
	QTime time;
	time.start();
	while (time.elapsed() < ms) {
		QApplication::processEvents();
		if (VisMotorInstance->IsEmgStop())return;
    }
}

void PluginMeasureProcess::SlotIoChange(QMap<QString, int> inStateMap)
{
    if (!VisMotorInstance->IsConnected()) return;
    if(VisMotorInstance->IsEmgStop()) return;
	if (!GlobalParam->systemParam.shieldParam.safeDoor &&VisMotorDataInstance->m_running) {
		/*if (inStateMap[In_SafeDoor]) {
			if (!VisMotorInstance->IsPause()) {
				ShowSystemLog(Log_Debug, QString(u8"安全门已打开，设备暂停"));
				event_AutoPause();
			}
		}
		else {
			if (VisMotorInstance->IsPause()) {
				ShowSystemLog(Log_Debug, QString(u8"安全门已关闭，设备恢复"));
				event_AutoResume();
			}
		}*/
	}
    //夹爪压力检测
    if (IO_OFF == inStateMap[IN_FeedHolderGripOvervoltage]||
        IO_OFF == inStateMap[IN_FilmtearGripOvervoltage]||
        IO_OFF == inStateMap[IN_FilmtearGripOvervoltage])
    {
//        if (!VisMotorInstance->IsEmgStop())
//            event_AutoEmg();
//        event_PopupErrInfo(QString(u8"夹爪触发压力报警，请检查是否已撞机"));
    }

    //设置三色灯闪烁
    if (m_manualFeeding && VisMotorDataInstance->m_running && !VisMotorInstance->IsPause() && !m_waitTray)
    {
        //手动模式下面绿灯闪烁（未处于暂停状态）
        m_ioState = (m_ioState == IO_ON ? IO_OFF : IO_ON);
        VisMotorInstance->SetIoOutput(Out_GreenLight, m_ioState);
    }
    //流水线入口缓存未检测到则判定为等盘状态
   /* if (VisMotorInstance->GetIoInput(IN_PipeLineInBlock) == IO_OFF)
        m_waitTray = true;
    else
        m_waitTray = false;*/
    if (m_waitTray && VisMotorDataInstance->m_running && !VisMotorInstance->IsPause())
    {
        //待料状态 黄绿闪烁
        m_ioState = (m_ioState == IO_ON ? IO_OFF : IO_ON);
        VisMotorInstance->SetIoOutput(Out_GreenLight, m_ioState);
        VisMotorInstance->SetIoOutput(Out_YellowLight, m_ioState);
    }
    else
    {
        if (!m_waitTray && VisMotorDataInstance->m_running && !VisMotorInstance->IsPause())     //未待料
        {
            VisMotorInstance->SetIoOutput(Out_YellowLight, IO_OFF);
            VisMotorInstance->SetIoOutput(Out_GreenLight, IO_ON);
        }
    }
}

int PluginMeasureProcess::event_PopupErrNotify(QString info, QStringList iocheck, QVector<int> normalLevel)
{
	if (VisMotorInstance->IsEmgStop())return HardWareErr;
    VisMotorInstance->SetIoOutput(Out_Buzzer, IO_ON);
	VisMotorInstance->SetPause();
	WaitTime(400);

	QString msg;
	if (iocheck.size() == 0)
		msg = info;
	else
		msg = info + QStringLiteral("\n是否继续作业流程？");
	while (1) {
         if (NonBlockingMsgBox::Ok == MsgBoxManager::instance().showWarning(nullptr, QStringLiteral("提示"),msg,NonBlockingMsgBox::Ok | NonBlockingMsgBox::Cancel)) {
			WaitTime(3000);
			bool flagCheck = true;
			for (int i = 0; i < iocheck.size(); i++) {
				if (i >= normalLevel.size() || VisMotorInstance->GetIoInput(iocheck.at(i)) != normalLevel[i]) {
					flagCheck = false;
					break;
				}
			}
			if (flagCheck) {
                if (VisMotorInstance->IsEmgStop())
                    return HardWareErr;
				WaitTime(50);
				VisMotorInstance->ResumeMove();
                VisMotorInstance->SetIoOutput(Out_Buzzer, IO_OFF);
                VisAppBus::sendEvent("DevException", false);
				return 0;
			}
		}
		else {
			ShowSystemLog(Log_Error, info);
            VisMotorInstance->SetIoOutput(Out_Buzzer, IO_OFF);
			VisMotorInstance->SetEmgStop();
            VisAppBus::sendEvent("DevException", false);
			return HardWareErr;
		}
	}
	return HardWareErr;
}

int PluginMeasureProcess::event_PopupErrInfo(QString info)
{
	ShowSystemLog(Log_Error, info);
	event_AutoEmg();
    MsgBoxManager::instance().showWarning(nullptr, QStringLiteral("错误信息"),info,NonBlockingMsgBox::Ok);
   
    return HardWareErr;
}

int PluginMeasureProcess::event_PopupUserMsgBox(QStringList listBtn, QString info)
{
    ShowSystemLog(Log_Info, info);
    VisMotorInstance->SetIoOutput(Out_Buzzer, IO_ON);
    int btnClick = MsgBoxManager::instance().showWarning(nullptr, QString(u8"提示信息"), info, listBtn);
    VisMotorInstance->SetIoOutput(Out_Buzzer, IO_OFF);
    return btnClick;
}


int PluginMeasureProcess::event_TestDirtyDetect(int station)
{
    return DirtyNode::event_TestDetect(station);
}


void PluginMeasureProcess::InitSlot()
{
	VisAppBus::subscibeEvent(this, "AutoReset");
    VisAppBus::subscibeEvent(this, "ClearModule");
	VisAppBus::subscibeEvent(this, "AutoStart");
	VisAppBus::subscibeEvent(this, "AutoPause");
	VisAppBus::subscibeEvent(this, "AutoResume");
	VisAppBus::subscibeEvent(this, "AutoEmg");
	VisAppBus::subscibeEvent(this, "ClearWarning");
	VisAppBus::subscibeEvent(this, "PopupWarning");
	VisAppBus::subscibeEvent(this, "PopupErrNotify");
	VisAppBus::subscibeEvent(this, "PopupErrInfo");
    VisAppBus::subscibeEvent(this, "PopupUserMsgBox");
	VisAppBus::subscibeEvent(this, "TestDirtyDetect");

	//复位信号，IO监测发出
	connect(VisSignalsManager::instance(), &VisSignalsManager::ResetSignal, this, [=] {
		if (VisMotorDataInstance->m_running)return;
		ShowSystemLog(Log_Debug, QString(u8"按钮复位"));
		sigMachineStatus(Homed);
		});
	//急停信号，IO监测发出
	connect(VisSignalsManager::instance(), &VisSignalsManager::EstopSignal, this, [=] {
		if (VisMotorInstance->GetIoInput(In_Emg) == IO_OFF && !VisMotorInstance->IsEmgStop()) {
			ShowSystemLog(Log_Error, QString(u8"按钮触发急停"));
			event_AutoEmg();
			sigMachineStatus(EmergencyStop);
		}
		});
	//气压异常信号，IO监测发出
	connect(VisSignalsManager::instance(), &VisSignalsManager::PressureSignal, this, [=] {
		if (VisMotorInstance->GetIoInput(In_AirCheck) == IO_OFF && !VisMotorInstance->IsEmgStop()) {
			ShowSystemLog(Log_Error, QString(u8"气压异常急停"));
			event_AutoEmg();
			sigMachineStatus(EmergencyStop);
		}
		});
	//流程启动信号
	connect(VisSignalsManager::instance(), &VisSignalsManager::StartProcessSignal, this, [=] {
		if (!VisMotorDataInstance->m_running && VisMotorInstance->IsResetted())
		{
            sigMachineStatus(AutoRunning);
		}
		});
	//流程停止信号
	connect(VisSignalsManager::instance(), &VisSignalsManager::StopProcessSignal, this, [=] {
		if (VisMotorDataInstance->m_running)
		{
			
		}
		});

	//电机错误信号
	connect(VisMotorInstance, &VisMotorManager::ErrorMessageSignal, this, [=](QString info) {
		ShowSystemLog(Log_Error, info);
		event_AutoEmg();
		}, Qt::DirectConnection);
    //其他IO监测发出
    connect(VisSignalsManager::instance(), &VisSignalsManager::SigIoState, this, &PluginMeasureProcess::SlotIoChange);
}

int PluginMeasureProcess::ResetPreCheck()
{
	//检测夹爪
	/*QStringList listGrip, listGripName;
	listGrip << MotorHolderGripX << MotorPCBGripX << MotorCleanGripX << MotorTurntableFeedGripX << MotorTurntableBlankGripX
		<< MotorFilmtearGripX << MotorTurntableCleanGripX;
	listGripName << u8"上料壳体夹爪" << u8"上料PCB夹爪" << u8"流线PCB清洗" << u8"上料转盘" << u8"下料转盘"
		<< u8"撕膜搬运夹爪" << u8"转盘清洗夹爪";
	for (int i = 0; i < listGrip.size(); i++) {
		curSportState state;
		if (!VisMotorInstance->ReadCurSportState(listGrip.at(i), state)) {
			ShowSystemLog(Log_Error, QString(u8"获取%1夹取状态失败,不能复位").arg(listGripName.at(i)));
			return -1;
		}
		if (state == SPAORTSTOPANDNOCATCH) {
			ShowSystemLog(Log_Error, QString(u8"检测到%1存在产品,不能复位").arg(listGripName.at(i)));
			return -1;
		}
	}*/
	//检查取料电机是否安全位
	double pos = -10000;
	/*VisMotorInstance->GetCurPos(MotorHolderBoxY, pos);
	if (pos > VisMotorDataInstance->GetPosMap(HolderBoxMotorWait)[MotorHolderBoxY]) {
		ShowSystemLog(Log_Error, QString(u8"检测到取料电机不在等待位").arg(listGripName.at(i)));
	}*/

	return 0;
}

int PluginMeasureProcess::CheckIO()
{
	ShowSystemLog(Log_Info, QString(u8"检查IO状态"));
    if (GlobalParam->flagOffline)
        return 0;
	//检测夹爪
	/*QStringList listGrip, listGripName;
	listGrip << MotorHolderGripX << MotorPCBGripX << MotorCleanGripX << MotorTurntableFeedGripX << MotorTurntableBlankGripX
		<< MotorFilmtearGripX << MotorTurntableCleanGripX;
	listGripName << u8"上料壳体" << u8"上料PCB" << u8"流线PCB清洗" << u8"上料转盘" << u8"下料转盘"
		<< u8"撕膜搬运" << u8"转盘清洗";
	for (int i = 0; i < listGrip.size(); i++) {
		curSportState state;
		if (!VisMotorInstance->ReadCurSportState(listGrip.at(i), state)) {
			ShowSystemLog(Log_Error, QString(u8"获取%1夹爪夹取状态失败,不能生产").arg(listGripName.at(i)));
			return -1;
		}
		if (state == SPAORTSTOPANDNOCATCH) {
			ShowSystemLog(Log_Error, QString(u8"检测到%1夹爪存在产品,不能生产").arg(listGripName.at(i)));
			return -1;
		}
	}*/
	VisMotorInstance->SetIoOutput(Out_PipeLineBackFlowMotorPower, IO_ON);
	VisMotorInstance->SetIoOutput(Out_PipeLineMotorPower, IO_ON);
	IOLevel level1 = VisMotorInstance->GetIoInput(IN_PipeLineTransferL);
	IOLevel level2 = VisMotorInstance->GetIoInput(IN_PipeLineTransferR);
	if (level1 == IO_ON || level2 == IO_ON) {
		ShowSystemLog(Log_Error, QString(u8"流线接驳台有载具,请先清空"));
		return -1;
	}
	int nRes = VisMotorInstance->SetIoOutput(Out_PipeLineMotorR, IO_ON);
	if (nRes != 0)return nRes;
	QTime time;
	time.start();
	QStringList listCheckIO,listCheckIOName;
	listCheckIO << IN_PipeLinePCB << IN_PipeLineClean << IN_PipeLineTurntable;
	listCheckIOName << u8"上料PCB" << u8"清洗PCB" << u8"上料转盘";
	while (time.elapsed()<6000) {
		for (int i = 0; i < listCheckIO.size(); i++) {
			IOLevel level = VisMotorInstance->GetIoInput(listCheckIO.at(i));
			if (level == IO_ON) {
				ShowSystemLog(Log_Error, QString(u8"输送线%1处有载具,请先清空").arg(listCheckIOName.at(i)));
				return -1;
			}
		}
	}

	VisMotorInstance->SetIoOutput(Out_PipeLineMotorR, IO_OFF);
	return 0;
}

int PluginMeasureProcess::InitIO()
{
	ShowSystemLog(Log_Info, QString(u8"初始化IO"));
    if (GlobalParam->flagOffline)return 0;
	int nRes = 0;
	QStringList listInitIo, listInfo;
	QVector<IOLevel>vecIoLevel;
	//指示灯
    VisMotorInstance->SetIoOutput(Out_YellowLight, IO_OFF);
    VisMotorInstance->SetIoOutput(Out_RedLight, IO_OFF);
    VisMotorInstance->SetIoOutput(Out_GreenLight, IO_ON);
    VisMotorInstance->SetIoOutput(Out_Buzzer, IO_OFF);
	VisMotorInstance->SetIoOutput(Out_Start1, IO_ON);
    VisMotorInstance->SetIoOutput(Out_Start2, IO_ON);
    
	//升流线阻挡、降顶升
	for (int i = EmptyBuf; i <= FeedTurntable; i++) {
		nRes = CylinderCtrl::instance()->event_SetBlockUp((TrayFunc)i, true);
		if (nRes != 0) return nRes;
		nRes = CylinderCtrl::instance()->event_SetPushUp((TrayFunc)i, false);
		if (nRes != 0) return nRes;
	}
	//缩回接驳台
	VisMotorInstance->SetDoubleIoOutput(Out_TransferBack, IO_ON, Out_TransferExend, IO_OFF);
	//上升PCB清洗夹爪
	for (int i = 0; i < 2; i++) {
		nRes = CylinderCtrl::instance()->event_SetCleanPCBGripUp(0, true);
		if (nRes != 0) return nRes;
	}

	return nRes;
}

int PluginMeasureProcess::CheckDevicePos()
{
	//检查取料电机是否安全位
	double pos = -10000;
	/*VisMotorInstance->GetCurPos(MotorHolderBoxY, pos);
	if (pos > VisMotorDataInstance->GetPosMap(HolderBoxMotorWait)[MotorHolderBoxY]) {
		ShowSystemLog(Log_Error, QString(u8"检测到取料电机不在等待位").arg(listGripName.at(i)));
	}*/

	return 0;
}

int PluginMeasureProcess::InitDevicePos()
{
	ShowSystemLog(Log_Info, QString(u8"初始化电机位置"));
    //夹爪到安全Z
    QMap<QString, double> mapGroup = VisMotorDataInstance->GetPosMap(HolderGripSafe);
    mapGroup = mapGroup.unite(VisMotorDataInstance->GetPosMap(PCBGripSafe));
    mapGroup = mapGroup.unite(VisMotorDataInstance->GetPosMap(TurntableFeedGripSafe));
    mapGroup = mapGroup.unite(VisMotorDataInstance->GetPosMap(FilmTearGripSafe));
    int nRes = VisMotorInstance->MoveAbsGroup(mapGroup);
    if (nRes != 0) return nRes;

    //龙门到等待位
	mapGroup = VisMotorDataInstance->GetPosMap(HolderScanCode);   //壳体
    mapGroup.remove(MotorHolderGantryZ);
	mapGroup = mapGroup.unite(VisMotorDataInstance->GetPosMap(PCBScanCode));  //PCB
    mapGroup.remove(MotorPCBGantryZ);
    mapGroup = mapGroup.unite(VisMotorDataInstance->GetPosMap(PipeLineCleanPCB_Grab));  //流线清洗
    mapGroup = mapGroup.unite(VisMotorDataInstance->GetPosMap(TurntableCleanPCB_Grab));  //转盘清洗

	mapGroup = mapGroup.unite(VisMotorDataInstance->GetPosMap(TurntablePipeLineGrab));  //转盘上下料龙门
	mapGroup.remove(MotorTurntableGantryZ);
	double initAngle = VisMotorDataInstance->GetPosMap(TurntableInitPos)[MotorTurntableR]; //转盘
	int curFixture = 0;
	for (int i = 0; i < 4; i++) {
		if (!GlobalParam->systemParam.shieldParam.turntable[i]) {
			curFixture = i;
			break;
		}
	}
	double  rotateAngle = curFixture * 90 + initAngle;
	mapGroup[MotorTurntableR] = rotateAngle;

	mapGroup = mapGroup.unite(VisMotorDataInstance->GetPosMap(FileTearFeed));  //撕膜龙门
	mapGroup.remove(MotorFilmtearGantryZ);
    mapGroup[MotorFilmtearX] = VisMotorDataInstance->GetPosMap(FileTearWork)[MotorFilmtearX];
    mapGroup[MotorDirtyY] = VisMotorDataInstance->GetPosMap(DetectDirty)[MotorDirtyY];     //脏污
	nRes = VisMotorInstance->MoveAbsGroup(mapGroup);
	if (nRes != 0) return nRes;

    return nRes;
}


int PluginMeasureProcess::DoClearModule()
{
    return 0;
}

int PluginMeasureProcess::event_SetProductEnd(bool end)
{
    m_productEnd = end;
    return 0;
}

int PluginMeasureProcess::event_GetProductEnd(bool& end)
{
    end = m_productEnd;
    return 0;
}

