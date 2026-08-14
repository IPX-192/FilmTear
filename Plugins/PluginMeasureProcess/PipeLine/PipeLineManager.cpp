#include "PipeLineManager.h"
#include <QApplication>
#include <QTime>
#include "VisAppBus.h"
#include "VisMotorManager.h"
#include "VisAppThreadPool.h"
#include "ParamManager.h"
#include "UdpClient.h"
#include "UdpServer.h"
#include "CylinderCtrl.h"
#include <QtCore/QMetaType>
Q_DECLARE_METATYPE(TrayFunc)
Q_DECLARE_METATYPE(TrayInfo)
Q_DECLARE_METATYPE(QVector<TrayFunc>)
Q_DECLARE_METATYPE(QVector<bool>)
Q_DECLARE_METATYPE(QVector<TrayInfo>)

using namespace  VisMotorToolSpace;
PipeLineManager::PipeLineManager(QObject *parent) : QObject(parent)
{
    m_udpClient=new UdpClient(this);
    m_udpServer=new UdpServer(this);

	for (int i = 0; i < 2; i++) {
		m_pineLineUseCount[i] = 0;
    }
    m_mapTrayName[EmptyBuf]=u8"空盘缓存位";
    m_mapTrayName[FeedHolder]=u8"上料壳体";
    m_mapTrayName[FeedPCB]=u8"上料PCB";
    m_mapTrayName[PCBClean]=u8"PCB清洗";
    m_mapTrayName[FeedTurntable]=u8"转盘上料";
	VisAppBus::subscibeEvent(this, "SetPipeLineMove");
    VisAppBus::subscibeEvent(this,"BlankPipeLineTray");
    VisAppBus::subscibeEvent(this,"SetBackFlowReady");
	VisAppBus::subscibeEvent(this, "PipeLineOutTransEnd");
    VisAppBus::subscibeEvent(this, "RefreshAllTrayMap");
    qRegisterMetaType<TrayFunc>("TrayFunc");
    qRegisterMetaType<QVector<TrayFunc>>();
    qRegisterMetaType<QVector<bool>>();
    qRegisterMetaType<QVector<TrayInfo>>();

}

PipeLineManager::~PipeLineManager()
{
    setStatus(STOP);
    {
        std::unique_lock<std::mutex> locker(m_mutex);
        m_condition.notify_one();
    }

    stop();
}

int PipeLineManager::Init()
{
    if (!m_udpServer->Start())return -1;
   // if (!m_udpClient->Start(GlobalParam->systemParam.preDeviceIp))return -1;
	for (int i = 0; i < 2; i++) {
		m_pineLineUseCount[i] = 0;
	}
    for(int i=EmptyBuf;i<=FeedTurntable;i++)
		m_mapExistTrayInfo[(TrayFunc)i].first=false;
    start();

    return 0;
}

bool PipeLineManager::doTask()
{
    // 阻塞等待：停机 或 有待移托盘任务
    std::unique_lock<std::mutex> locker(m_mutex);
    m_condition.wait(locker, [this] {
        return status == STOP || !m_listTrayTask.empty();
        });
    if (status == STOP) {
        return false;
    }
	if (VisMotorInstance->IsEmgStop())return false;
    if (m_listTrayTask.empty()) return true;
    QVector<TrayFunc> listTrayTask=m_listTrayTask;
    for(int i=0;i<listTrayTask.size();i++){
        TrayFunc src=listTrayTask[i];
        TrayFunc dst=(TrayFunc)(src+1);
        //下个位置空闲
        if(m_mapExistTrayInfo[dst].first==false){
           GlobalThreadPool->Commit(std::bind(&PipeLineManager::MoveTray,this,src));
           m_listTrayTask.removeOne(src);
        }
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    return true;
}

int PipeLineManager::MoveTray(TrayFunc src)
{
	int nRes = 0;
    TrayFunc dst=(TrayFunc)(src+1);
    ShowLog(Log_PipeLine, 0, Log_Info, QString(u8"%1处载具开始移动到%2处").arg(m_mapTrayName[src]).arg(m_mapTrayName[dst]));
	if (src == EmptyBuf) {
		nRes = MoveTrayToHolder();
		if (nRes != 0)return nRes;
	}
    else if(src==FeedHolder){
        nRes = MoveTrayToFeedPCB();
        if (nRes != 0)return nRes;
    }
    else if(src==FeedPCB){
		nRes = MoveTrayToCleanPCB();
		if (nRes != 0)return nRes;
    }
    else if(src==PCBClean){
		nRes = MoveTrayToTurntable();
		if (nRes != 0)return nRes;
    }

    ShowLog(Log_PipeLine, 0, Log_Info, QString(u8"载具到达%1处").arg(m_mapTrayName[dst]));
	m_mapExistTrayInfo[dst].second = m_mapExistTrayInfo[src].second;

    // 托盘到达 PCB 上料工位时读取 RFID 托盘码
    if (dst == FeedPCB)
    {
        QString barCode;
        int codeLen = GlobalParam->hardwareParam.trayRfidDebugParam.codeLength;
        VisAppBus::sendEvent("GetModbusTrayCode", (int)dst, std::ref(barCode), codeLen);
        m_mapExistTrayInfo[dst].second.trayCode = barCode;
        ShowLog(Log_PipeLine, 0, Log_Info, QString(u8"PCB工位读取托盘码:%1").arg(barCode));

        // 和上游 UDP 传来的托盘码比对
        QString upTrayCode = m_mapExistTrayInfo[src].second.trayCode;
        if (!upTrayCode.isEmpty() && barCode != upTrayCode)
        {
            ShowLog(Log_PipeLine, 0, Log_Error,
                QString(u8"托盘码比对不一致! RFID读取:%1 , 上游下发:%2").arg(barCode).arg(upTrayCode));
        }
    }

	m_mapExistTrayInfo[dst].first = true;
	m_mapExistTrayInfo[src].second = TrayInfo();
	m_mapExistTrayInfo[src].first = false;

    sigPipeLineTrayReady(dst, m_mapExistTrayInfo[dst].second);

    return 0;
}

int PipeLineManager::MoveTrayToHolder()
{
    //接驳台到回流线
    int nRes = VisMotorInstance->MovePositionAbs(BackFlowTransfer);
	if (nRes != 0)return nRes;
	//降阻挡
	nRes = CylinderCtrl::instance()->event_SetBlockUp(EmptyBuf, false);
	if (nRes != 0)return nRes;
	//流水线动作
	nRes = VisMotorInstance->MotorMoveAbs(MotorTransferX, -1, false, true);
	if (nRes != 0)return nRes;
	nRes = VisMotorInstance->SetIoOutput(Out_PipeLineBackFlowMotorL, IO_ON);
	if (nRes != 0)return nRes;
	//等待托盘流到接驳线左边
    while (1) {
        if (VisMotorInstance->SelectIoInput(IN_PipeLineTransferL, IO_ON, 20000))
            break;
        if (VisMotorInstance->IsEmgStop()) return HardWareErr;
        QString errInfo = QString(u8"20秒内接驳线左边没有检测到托盘信号,请放盘后点确定");
        ShowLog(Log_PipeLine, 0, Log_Error, errInfo);
        int nRet = VisAppBus::sendEvent("PopupWarning", errInfo);
        if (nRet != 0) return -1;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	//停止流水线动作
    VisMotorInstance->StopContinuous(MotorTransferX);
	nRes = VisMotorInstance->SetIoOutput(Out_PipeLineBackFlowMotorL, IO_OFF);
	if (nRes != 0)return nRes;
	//上升阻挡
	nRes = CylinderCtrl::instance()->event_SetBlockUp(EmptyBuf, true);
	if (nRes != 0)return nRes;
    //接驳台到上料输送线
	nRes = VisMotorInstance->MovePositionAbs(FeedTransfer);
	if (nRes != 0)return nRes;
	sigSetBackFlowStatus(false);
    return 0;
}

int PipeLineManager::MoveTrayToFeedPCB()
{
	//启动输送流水线
    int nRes = event_SetPipeLineMove(true);
    if (nRes != 0)return nRes;
	nRes = VisMotorInstance->MotorMoveAbs(MotorTransferX, 1, false, true);
	if (nRes != 0)return nRes;
	//等待托盘流到上料PCB
	if (!VisMotorInstance->SelectIoInput(IN_PipeLinePCB, IO_ON, 15000)) {
		QString errInfo = QString(u8"15秒内流线上料PCB处没有检测到托盘信号");
		ShowLog(Log_PipeLine, 0, Log_Error, errInfo);
        nRes = VisAppBus::sendEvent("PopupErrNotify", errInfo);
		if (nRes != 0)return nRes;
	}
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	//停止流水线动作
	VisMotorInstance->StopContinuous(MotorTransferX);
	nRes = event_SetPipeLineMove(false);
	if (nRes != 0)return nRes;
    //顶升托盘
    nRes = CylinderCtrl::instance()->event_SetPushUp(FeedPCB, true);
    if (nRes != 0)return nRes;
	
    return 0;
}

int PipeLineManager::MoveTrayToCleanPCB()
{
	//下降顶升
	int nRes = CylinderCtrl::instance()->event_SetPushUp(FeedPCB, false);
	if (nRes != 0)return nRes;
    //下降阻挡
	nRes = CylinderCtrl::instance()->event_SetBlockUp(FeedPCB, false);
	if (nRes != 0)return nRes;
	//启动输送流水线
	nRes = event_SetPipeLineMove(true);
	if (nRes != 0)return nRes;
	//等待托盘流到清洗PCB
	if (!VisMotorInstance->SelectIoInput(IN_PipeLineClean, IO_ON, 20000)) {
		QString errInfo = QString(u8"20秒内流线清洗PCB处没有检测到托盘信号");
		ShowLog(Log_PipeLine, 0, Log_Error, errInfo);
		nRes = VisAppBus::sendEvent("PopupErrNotify", errInfo);
		if (nRes != 0)return nRes;
	}
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	//停止流水线动作
    nRes = event_SetPipeLineMove(false);
	if (nRes != 0)return nRes;
	//顶升托盘
	nRes = CylinderCtrl::instance()->event_SetPushUp(PCBClean, true);
	if (nRes != 0)return nRes;
	//上升阻挡
	nRes = CylinderCtrl::instance()->event_SetBlockUp(FeedPCB, true);
	if (nRes != 0)return nRes;
    return 0;
}

int PipeLineManager::MoveTrayToTurntable()
{
	//下降顶升
	int nRes = CylinderCtrl::instance()->event_SetPushUp(PCBClean, false);
	if (nRes != 0)return nRes;
	//下降阻挡
	nRes = CylinderCtrl::instance()->event_SetBlockUp(PCBClean, false);
	if (nRes != 0)return nRes;
	//启动输送流水线
	nRes = event_SetPipeLineMove(true);
	if (nRes != 0)return nRes;
	//等待托盘流到清洗PCB
	if (!VisMotorInstance->SelectIoInput(IN_PipeLineTurntable, IO_ON, 20000)) {
		QString errInfo = QString(u8"20秒内流线转盘处没有检测到托盘信号");
		ShowLog(Log_PipeLine, 0, Log_Error, errInfo);
		nRes = VisAppBus::sendEvent("PopupErrNotify", errInfo);
		if (nRes != 0)return nRes;
	}
    std::this_thread::sleep_for(std::chrono::milliseconds(1500));
	//停止流水线动作
    nRes = event_SetPipeLineMove(false);
	if (nRes != 0)return nRes;
	//顶升托盘
	nRes = CylinderCtrl::instance()->event_SetPushUp(FeedTurntable, true);
	if (nRes != 0)return nRes;
	//上升阻挡
	nRes = CylinderCtrl::instance()->event_SetBlockUp(PCBClean, true);
	if (nRes != 0)return nRes;
	return 0;
}

void PipeLineManager::sigPipeLineTrayReady(TrayFunc type, TrayInfo info)
{
    VisAppBus::sendEventDirect("PipeLineTrayReady",type, info);
}


void PipeLineManager::sigSetPipeLineOutStatus(bool exist, TrayInfo info)
{
	VisAppBus::sendEvent("SetPipeLineOutStatus", exist, info);
}

void PipeLineManager::sigSetBackFlowStatus(bool exist)
{
	VisAppBus::sendEventDirect("SetBackFlowStatus", exist);
}

int PipeLineManager::event_SetPipeLineMove(bool enable)
{
    int index = 0;
    m_pipeLineMutex.lock();
	int nRes = 0;
    m_pineLineUseCount[index] += (enable ? 1 : -1);
	if (m_pineLineUseCount[index] < 0) {
		m_pineLineUseCount[index] = 0;
	}
	if (m_pineLineUseCount[index] == 0) {
        nRes = VisMotorInstance->SetIoOutput(Out_PipeLineMotorR, IO_OFF);
	}
	else if (m_pineLineUseCount[index] == 1) {
        nRes = VisMotorInstance->SetIoOutput(Out_PipeLineMotorR, IO_ON);
	}
    m_pipeLineMutex.unlock();
    return nRes;
}

int PipeLineManager::event_SetBackFlowReady()
{
    ShowLog(Log_PipeLine, 0, Log_Info, QString(u8"%1处有载具").arg(m_mapTrayName[EmptyBuf]));
    std::unique_lock<std::mutex> locker(m_mutex);
    m_mapExistTrayInfo[EmptyBuf].first = true;
    m_listTrayTask.push_back(EmptyBuf);
    m_condition.notify_one();
    return 0;
}

int PipeLineManager::event_BlankPipeLineTray(TrayFunc type, TrayInfo info)
{
	m_mapExistTrayInfo[type].second = info;
    if (type == FeedTurntable) {
		ShowLog(Log_PipeLine, 0, Log_Info, QString(u8"请求%1处载具流走").arg(m_mapTrayName[type]));
		sigSetPipeLineOutStatus(true, info);
    }
    else {
		std::unique_lock<std::mutex> locker(m_mutex);
		m_listTrayTask.push_back(type);
		m_condition.notify_one();

    }

    return 0;
}

int PipeLineManager::event_PipeLineOutTransEnd()
{
	m_mapExistTrayInfo[FeedTurntable].second = TrayInfo();
	m_mapExistTrayInfo[FeedTurntable].first = false;
    return 0;
}

int PipeLineManager::event_RefreshAllTrayMap(QVector<TrayFunc>& vecFunc, QVector<bool>& vecHasTray, QVector<TrayInfo>& vecTrayData)
{
    // 清空输出容器
    vecFunc.clear();
    vecHasTray.clear();
    vecTrayData.clear();
    for (auto iter = m_mapExistTrayInfo.begin(); iter != m_mapExistTrayInfo.end(); ++iter)
    {
        TrayFunc func = iter.key();
        auto& pair = iter.value();
        vecFunc.push_back(func);
        vecHasTray.push_back(pair.first);
        vecTrayData.push_back(pair.second);
    }
    return 0;
}
