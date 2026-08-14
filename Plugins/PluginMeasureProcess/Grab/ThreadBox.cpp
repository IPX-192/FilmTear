#include "ThreadBox.h"
#include "VisMotorManager.h"
#include "VisMotorToolData.h"
#include "VisAppBus.h"
#include "CylinderCtrl.h"
#include <QDebug>
using namespace VisMotorToolSpace;

ThreadBox::ThreadBox(TrayFunc type,QObject *parent) :
    m_funcType(type),
    QObject(parent)
{
    m_logType = (m_funcType == FeedHolder) ? Log_GrabHolder : Log_GrabPCB;
    VisAppBus::subscibeEvent(this, "InTrayTask");
    VisAppBus::subscibeEvent(this, "BlankTray");
    VisAppBus::subscibeEvent(this, "SupplyMaterial");
    VisAppBus::subscibeEvent(this, "InTrayTaskLayer");
    VisAppBus::subscibeEvent(this, "BlankTrayLayer");
}

ThreadBox::~ThreadBox()
{
    setStatus(STOP);
    {
        std::unique_lock<std::mutex> locker(m_mutex);
        m_condition.notify_one();
    }
    stop();
}

void ThreadBox::UpdateBoxParam()
{
    FeedBoxParam boxParam;
    boxParam.boxName = (m_funcType == FeedHolder) ? u8"壳体料仓" : u8"PCB料仓";
    boxParam.nameAxisTrayY = (m_funcType == FeedHolder) ? MotorHolderBoxY : MotorPCBBoxY;
    boxParam.nameAxisZ = (m_funcType == FeedHolder) ? MotorHolderBoxZ : MotorPCBBoxZ;
    boxParam.trayWaitPos = (m_funcType == FeedHolder) ? HolderBoxMotorWait : PCBBoxMotorWait;

    boxParam.trayPosIn_Out[InTray] = VisMotorDataInstance->GetPosMap((m_funcType == FeedHolder) ? HolderBoxGrabTray : PCBBoxGrabTray)[boxParam.nameAxisTrayY];
    boxParam.trayPosIn_Out[OutTray] = VisMotorDataInstance->GetPosMap((m_funcType == FeedHolder) ? HolderBoxMotorWait : PCBBoxMotorWait)[boxParam.nameAxisTrayY];

    // 首层Z基准值
    double baseCheckTrayZ  = VisMotorDataInstance->GetPosMap((m_funcType == FeedHolder) ? HolderBoxCheckTray : PCBBoxCheckTray)[boxParam.nameAxisZ];
    double baseGrabTrayZ   = VisMotorDataInstance->GetPosMap((m_funcType == FeedHolder) ? HolderBoxGrabTray  : PCBBoxGrabTray)[boxParam.nameAxisZ];
    double baseTrayTouchZ  = VisMotorDataInstance->GetPosMap((m_funcType == FeedHolder) ? HolderTrayTouch    : PCBTrayTouch)[boxParam.nameAxisZ];
    double baseTrayUpZ     = VisMotorDataInstance->GetPosMap((m_funcType == FeedHolder) ? HolderBoxTrayUp    : PCBBoxTrayUp)[boxParam.nameAxisZ];

    // 层间Z间距（每层向下偏移，负值,来自系统设置-料箱参数）
    boxParam.layerPitchZ = (m_funcType == FeedHolder) ? GlobalParam->hardwareParam.holderBoxPitch.toDouble() : GlobalParam->hardwareParam.pcbBoxPitch.toDouble();
    // Z轴下限（硬件限位，超出clamp到此值）
    const double zLimitMin = (m_funcType == FeedHolder) ? -439.1 : -1e9;

    for (int i = 0; i < boxParam.layers; i++) {
        double zOffset = i * boxParam.layerPitchZ;
        auto clampZ = [&](double z) { return z < zLimitMin ? zLimitMin : z; };
        boxParam.vecCheckTrayExistZ.push_back(clampZ(baseCheckTrayZ + zOffset));
        boxParam.vecBoxUpPosZ.push_back(clampZ(baseGrabTrayZ + zOffset));
        boxParam.vecFixTrayPosZ.push_back(clampZ(baseTrayTouchZ + zOffset));
        boxParam.vecMotorBackZ.push_back(clampZ(baseTrayUpZ + zOffset));
    }
    boxParam.checkBoxExist = (m_funcType == FeedHolder) ? IN_HolderBoxReady : IN_PCBBoxReady;
    boxParam.checkBoxTrayExist = (m_funcType == FeedHolder) ? IN_HolderBoxTray : IN_PCBBoxTray;
    if (m_funcType == FeedHolder)
        boxParam.checkTray << IN_HolderTrayTilt1 << IN_HolderTrayTilt2;
    else
        boxParam.checkTray << IN_PCBTrayTilt1 << IN_PCBTrayTilt2;

    m_boxParam = boxParam;
}

void ThreadBox::InitParam()
{
    UpdateBoxParam();
    m_flagTaskInTray = false;
    m_curLayer = 0;
    m_listTask.clear();
    m_manualLayer = -1;
    start();
}

bool ThreadBox::doTask()
{
    std::unique_lock<std::mutex> locker(m_mutex);
    m_condition.wait(locker, [this] {
        return status == STOP || !m_listTask.empty() || m_flagTaskInTray;
    });
    if (status == STOP) {
        return false;
    }
    if (m_listTask.empty() && (!m_flagTaskInTray)) return true;

    int nRes = 0;
    //进盘
    if (m_flagTaskInTray) {
        locker.unlock();
        bool existTray = false;
        int layer = 0;
        nRes = CheckFeedBox(existTray, layer);
        if (nRes != 0) return false;
        if (!existTray) {
            pause();
            return true;
        }
        nRes = ProcessTray(std::make_pair(layer, InTray));
        if (nRes != 0) return false;
        m_flagTaskInTray = false;
        sigTrayReady(m_funcType);
    }
    else {   //出盘
        auto curTask = std::move(m_listTask.front());
        m_listTask.pop_front();
        locker.unlock();
        nRes = ProcessTray(std::make_pair(curTask, OutTray));
        if (nRes != 0) return false;
        //自动进盘
        m_flagTaskInTray = true;
    }
    return true;
}

int ThreadBox::CheckFeedBox(bool& existTray, int& layer)
{
    existTray = false;
    ShowLog(m_logType, 0, Log_Info, QString(u8"%1进行可用料盘检测").arg(m_boxParam.boxName));
    int level;
    //检查料仓是否存在
    level = IO_ON;
    m_errInfo = QString(u8"%1未检测到存在信号").arg(m_boxParam.boxName);
    int  nRes = CylinderCtrl::instance()->CheckIoState(m_boxParam.checkBoxExist, level, m_errInfo);
    if (nRes != 0){ShowLog(m_logType,0,Log_Error,m_errInfo);return nRes;}

    //手动指定层：先移取料电机到等待位，再只检查该层（不依赖自动流程的QueryInPos）
    if (m_manualLayer >= 0) {
        int targetLayer = m_manualLayer;
        m_manualLayer = -1;
        //移动取料电机到等待位
        nRes = VisMotorInstance->MotorMoveAbs(m_boxParam.nameAxisTrayY, m_boxParam.trayPosIn_Out[OutTray]);
        if (nRes != 0){
            m_errInfo = QString(u8"%1取料电机Y移到等待位失败,errcode=%2").arg(m_boxParam.boxName).arg(nRes);
            ShowLog(m_logType,0,Log_Error,m_errInfo);return nRes;
        }
        m_curLayer = targetLayer;
        nRes = VisMotorInstance->MotorMoveAbs(m_boxParam.nameAxisZ, m_boxParam.vecCheckTrayExistZ[targetLayer]);
        if (nRes != 0){
            m_errInfo = QString(u8"%1Z移到检查位失败,errcode=%2").arg(m_boxParam.boxName).arg(nRes);
            ShowLog(m_logType,0,Log_Error,m_errInfo);return nRes;
        }
        level = VisMotorInstance->GetIoInput(m_boxParam.checkBoxTrayExist);
        if (GlobalParam->flagOffline || GlobalParam->emptyRun)
            level = IO_ON;
        if (level == IO_OFF) {
            ShowLog(m_logType, 0, Log_Info, QString(u8"%1第%2层无料盘").arg(m_boxParam.boxName).arg(targetLayer + 1));
            sigBoxTrayStatus(m_funcType, targetLayer, QString(u8"无盘"));
            return 0;
        }
        ShowLog(m_logType, 0, Log_Info, QString(u8"%1第%2层料盘可用").arg(m_boxParam.boxName).arg(targetLayer + 1));
        sigBoxTrayStatus(m_funcType, targetLayer, QString(u8"有盘"));
        existTray = true;
        layer = targetLayer;
        m_curLayer = targetLayer + 1;
        m_flagNoMaterial = false;
        return 0;
    }

    //自动模式：检查取料电机是否在等待位
    if (!QueryInPos(m_boxParam.trayWaitPos)) {
        m_errInfo = QString(u8"%1上下移动时,取料电机未在等待位").arg(m_boxParam.boxName);
        ShowLog(m_logType,0,Log_Error,m_errInfo);
        return HardWareErr;
    }
    //料仓此前测试无料
    if (m_flagNoMaterial) {
        m_curLayer = 0;
        nRes = VisMotorInstance->MotorMoveAbs(m_boxParam.nameAxisZ, m_boxParam.vecCheckTrayExistZ[m_curLayer]);
        if (nRes != 0)return nRes;
        level = VisMotorInstance->GetIoInput(m_boxParam.checkBoxTrayExist);
        if (GlobalParam->flagOffline)
            level = IO_ON;
        if (level == IO_OFF) {
            ShowSystemLog(Log_Debug, QString(u8"%1首层未检测到料盘,无法补料").arg(m_boxParam.boxName));
            return 0;
        }
        m_flagNoMaterial = false;
    }
    //检查每一层是否有料
    for (; m_curLayer < m_boxParam.layers; m_curLayer++)
    {
        nRes = VisMotorInstance->MotorMoveAbs(m_boxParam.nameAxisZ, m_boxParam.vecCheckTrayExistZ[m_curLayer]);
        if (nRes != 0)return nRes;
        //检查对应层托盘信号
        level = VisMotorInstance->GetIoInput(m_boxParam.checkBoxTrayExist);
        if (GlobalParam->flagOffline)
            level = IO_ON;
        if (level==IO_OFF) {
            ShowLog(m_logType, 0, Log_Info, QString(u8"%1未检测到第%2层料盘").arg(m_boxParam.boxName).arg(m_curLayer + 1));
            sigBoxTrayStatus(m_funcType, m_curLayer, QString(u8"无盘"));
            continue;
        }
        ShowLog(m_logType, 0, Log_Info, QString(u8"%1第%2层料盘可用").arg(m_boxParam.boxName).arg(m_curLayer + 1));
        sigBoxTrayStatus(m_funcType, m_curLayer, QString(u8"有盘"));
        existTray = true;
        layer = m_curLayer;
        m_curLayer++;
        m_flagNoMaterial = false;
        return 0;
    }
    //无料状态
    ShowLog(m_logType, 0, Log_Debug, QString(u8"%1无可用料盘").arg(m_boxParam.boxName));
    m_flagNoMaterial = true;
    m_curLayer = 0;
    VisAppBus::sendEvent("PopupErrInfo", QString(u8"%1没有可用料盘"));

    return 0;
}


int ThreadBox::ProcessTray(std::pair<int, TrayOperate> trayTask)
{
    if (trayTask.second == OutTray)
        ShowLog(m_logType, 0, Log_Info, QString(u8"退料箱第%2层料盘").arg(trayTask.first + 1));
    else
        ShowLog(m_logType, 0, Log_Info, QString(u8"取料箱第%2层料盘").arg(trayTask.first + 1));

    int layer = trayTask.first;
    //取料电机到安全位(Y轴到等待位)
    int nRes = VisMotorInstance->MotorMoveAbs(m_boxParam.nameAxisTrayY, m_boxParam.trayPosIn_Out[OutTray]);
    if (nRes != 0){m_errInfo=QString(u8"%1取料电机Y移到等待位失败,errcode=%2").arg(m_boxParam.boxName).arg(nRes);ShowLog(m_logType,0,Log_Error,m_errInfo);return HardWareErr;}

    //检查取料电机托盘
    /*if (CheckSlantIO(feedbox.io_checkTray, level)) {
        m_errInfo = QString(u8"%1取料电机托盘倾斜").arg(feedbox.boxName);
        return HardWareErr;
    }*/
    int level;
    if (trayTask.second == OutTray) {
        level = IO_ON;
        m_errInfo = QString(u8"%1取料电机未检测到托盘,请检查确认").arg(m_boxParam.boxName);
    }
    else if (trayTask.second == InTray) {
        level = IO_OFF;
        m_errInfo = QString(u8"%1取料电机检测到存在托盘,请检查确认").arg(m_boxParam.boxName);
    }
    if (GlobalParam->emptyRun && trayTask.second == InTray) {
        //空跑:取料电机有盘则视为已取到,直接成功;无盘则正常进料箱
        bool hasTray = true;
        for (const auto& io : m_boxParam.checkTray) {
            if (VisMotorInstance->GetIoInput(io) != IO_ON)
                hasTray = false;
        }
        if (hasTray) {
            ShowLog(m_logType, 0, Log_Info, QString(u8"空跑模式:取料电机已有托盘,跳过进料箱"));
            return 0;
        }
    }
    else {
        nRes = CylinderCtrl::instance()->CheckIoState(m_boxParam.checkTray, level, m_errInfo);
        if (nRes != 0)return HardWareErr;
    }

    //检查料仓是否倾斜
    /*if (CheckSlantIO(feedbox.io_checkBox, level)) {
        m_errInfo = QString(u8"%1取料电机托盘倾斜").arg(feedbox.boxName);
        return HardWareErr;
    }*/
    nRes = VisMotorInstance->MotorMoveAbs(m_boxParam.nameAxisZ, m_boxParam.vecCheckTrayExistZ[layer]);
    if (nRes != 0){m_errInfo=QString(u8"%1Z移到检查位失败,errcode=%2").arg(m_boxParam.boxName).arg(nRes);ShowLog(m_logType,0,Log_Error,m_errInfo);return nRes;}
    if (trayTask.second == InTray) {
        //检查当前层是否有盘
        level = IO_ON;
        m_errInfo = QString(u8"向%1第%2层取盘时，未检查到托盘信号").arg(m_boxParam.boxName).arg(layer + 1);
        nRes = CylinderCtrl::instance()->CheckIoState(m_boxParam.checkBoxTrayExist, level, m_errInfo);
        if (nRes != 0)return nRes;
        nRes = VisMotorInstance->MotorMoveAbs(m_boxParam.nameAxisZ, m_boxParam.vecBoxUpPosZ[layer]);
        if (nRes != 0){m_errInfo=QString(u8"%1Z移到进盘位失败,errcode=%2").arg(m_boxParam.boxName).arg(nRes);ShowLog(m_logType,0,Log_Error,m_errInfo);return nRes;}
    }
    else {
        //检查当前层是否有盘
        level = IO_OFF;
        m_errInfo = QString(u8"向%1第%2层退盘时，检查到已存在托盘").arg(m_boxParam.boxName).arg(layer + 1);
        nRes = CylinderCtrl::instance()->CheckIoState(m_boxParam.checkBoxTrayExist, level, m_errInfo);
        if (nRes != 0)return nRes;
        nRes = VisMotorInstance->MotorMoveAbs(m_boxParam.nameAxisZ, m_boxParam.vecMotorBackZ[layer]);
        if (nRes != 0){m_errInfo=QString(u8"%1Z移到退盘起始位失败,errcode=%2").arg(m_boxParam.boxName).arg(nRes);ShowLog(m_logType,0,Log_Error,m_errInfo);return nRes;}
    }

    //取料盘电机到料箱
    nRes = VisMotorInstance->MotorMoveAbs(m_boxParam.nameAxisTrayY, m_boxParam.trayPosIn_Out[InTray]);
    if (nRes != 0){m_errInfo=QString(u8"%1取料电机Y进料仓失败,errcode=%2").arg(m_boxParam.boxName).arg(nRes);ShowLog(m_logType,0,Log_Error,m_errInfo);return nRes;}
    //到料箱固定Z
    nRes = VisMotorInstance->MotorMoveAbs(m_boxParam.nameAxisZ, m_boxParam.vecFixTrayPosZ[layer]);
    if (nRes != 0){m_errInfo=QString(u8"%1Z到固定位失败,errcode=%2").arg(m_boxParam.boxName).arg(nRes);ShowLog(m_logType,0,Log_Error,m_errInfo);return nRes;}
    if (trayTask.second == InTray) {
        nRes = VisMotorInstance->MotorMoveAbs(m_boxParam.nameAxisZ, m_boxParam.vecMotorBackZ[layer]);
        if (nRes != 0){m_errInfo=QString(u8"%1Z托起失败,errcode=%2").arg(m_boxParam.boxName).arg(nRes);ShowLog(m_logType,0,Log_Error,m_errInfo);return nRes;}
        //检查取料电机托盘
        level = IO_ON;
        m_errInfo = QString(u8"%1第%2层在托盘退出位没检测到托盘信号").arg(m_boxParam.boxName).arg(layer + 1);
        nRes = CylinderCtrl::instance()->CheckIoState(m_boxParam.checkTray, level, m_errInfo);
        if (nRes != 0)return nRes;
    }
    else {
        nRes = VisMotorInstance->MotorMoveAbs(m_boxParam.nameAxisZ, m_boxParam.vecBoxUpPosZ[layer]);
        if (nRes != 0){m_errInfo=QString(u8"%1Z回到进盘位失败,errcode=%2").arg(m_boxParam.boxName).arg(nRes);ShowLog(m_logType,0,Log_Error,m_errInfo);return nRes;}
    }

    //取料电机退出料仓
    if (trayTask.second == InTray) {
        nRes = VisMotorInstance->MotorMoveAbs(m_boxParam.nameAxisTrayY, m_boxParam.trayPosIn_Out[OutTray]);
        if (nRes != 0){m_errInfo=QString(u8"%1取料电机Y退出料仓失败,errcode=%2").arg(m_boxParam.boxName).arg(nRes);ShowLog(m_logType,0,Log_Error,m_errInfo);return nRes;}
        level = IO_ON;
        m_errInfo = QString(u8"%1取料电机取料后没检测到托盘信号").arg(m_boxParam.boxName);
        nRes = CylinderCtrl::instance()->CheckIoState(m_boxParam.checkTray, level, m_errInfo);
        if (nRes != 0)return nRes;
    }
    else {//到安全位
        nRes = VisMotorInstance->MotorMoveAbs(m_boxParam.nameAxisTrayY, m_boxParam.trayPosIn_Out[OutTray]);
        if (nRes != 0){m_errInfo=QString(u8"%1取料电机Y回到等待位失败,errcode=%2").arg(m_boxParam.boxName).arg(nRes);ShowLog(m_logType,0,Log_Error,m_errInfo);return nRes;}
        //检查料仓托盘
        nRes = VisMotorInstance->MotorMoveAbs(m_boxParam.nameAxisZ, m_boxParam.vecCheckTrayExistZ[layer]);
        if (nRes != 0){m_errInfo=QString(u8"%1Z回到检查位失败,errcode=%2").arg(m_boxParam.boxName).arg(nRes);ShowLog(m_logType,0,Log_Error,m_errInfo);return nRes;}
        level = IO_ON;
        m_errInfo = QString(u8"%1退盘后第%2层没检测到托盘信号").arg(m_boxParam.boxName).arg(layer + 1);
        nRes = CylinderCtrl::instance()->CheckIoState(m_boxParam.checkBoxTrayExist, level, m_errInfo);
        if (nRes != 0)return nRes;
        sigBoxTrayStatus(m_funcType, m_curLayer - 1, QString(u8"空盘"));
    }
    return 0;
}

bool ThreadBox::QueryInPos(QString posname)
{
    QMap<QString, PointValue>mapPosInfo = VisMotorDataInstance->GetPosInfo(posname);
    QMap<QString, double> mapMoveGroup;
    for (auto it = mapPosInfo.begin(); it != mapPosInfo.end(); ++it) {
        mapMoveGroup[it->stAxleName] = it->stValue;
    }
    bool flagInPos = false;
    VisMotorInstance->QueryMotorInPos(mapMoveGroup, flagInPos);
    return  flagInPos;
}


void ThreadBox::sigBoxTrayStatus(TrayFunc type, int layer, QString info)
{
    VisAppBus::sendEvent("BoxTrayStatus", type, layer, info);
}

void ThreadBox::sigTrayReady(TrayFunc type)
{
    VisAppBus::sendEventDirect("TrayReady", type);
}

//流水线要料，开启自动遍历料仓出料
int ThreadBox::event_InTrayTask(TrayFunc type)
{
    if (type != m_funcType)return 0;
    std::unique_lock<std::mutex> locker(m_mutex);
    m_flagTaskInTray = true;
    m_condition.notify_one();
}

//抓取模块托盘流走，自动退盘；
int ThreadBox::event_BlankTray(TrayFunc type)
{
    if (type != m_funcType)return 0;
    std::lock_guard<std::mutex> locker(m_mutex);
    int targetLayer = m_curLayer - 1;
    if(targetLayer >= 0)
    {
        m_listTask.push_back(targetLayer);
    }
    m_condition.notify_one();
    return 0;
}

//补料后唤醒料仓线程重新检测
int ThreadBox::event_SupplyMaterial()
{
    if (!m_flagNoMaterial)return 0;
    start();
    std::unique_lock<std::mutex> locker(m_mutex);
    m_condition.notify_one();

    return 0;
}

// 手动指定层拉料（走自动流程完整检查路径）
int ThreadBox::event_InTrayTaskLayer(TrayFunc type,int layer)
{
    if (type != m_funcType)
        return 0;

    UpdateBoxParam();
    std::unique_lock<std::mutex> locker(m_mutex);
    m_listTask.clear();
    m_manualLayer = layer;
    m_flagTaskInTray = true;
    m_condition.notify_one();
    start();
    return 0;
}

// 手动指定层退料
int ThreadBox::event_BlankTrayLayer(TrayFunc type,int layer)
{
    if (type != m_funcType)
        return 0;
    UpdateBoxParam();
    std::lock_guard<std::mutex> locker(m_mutex);
    m_listTask.clear();
    m_listTask.push_back(layer);
    m_condition.notify_one();
    start();
    return 0;
}
