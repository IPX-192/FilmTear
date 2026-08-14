#include "ThreadFeed.h"
#include "VisAppBus.h"
#include "ParamManager.h"
#include "VisMotorManager.h"
#include "VisMotorToolData.h"
#include "VisCameraTool.h"
#include "CylinderCtrl.h"
#include "opencv2/opencv.hpp"
#include "VisAppThreadPool.h"
#include "ThreadBox.h"
#include <QDebug>

using namespace  VisMotorToolSpace;

ThreadFeed::ThreadFeed(TrayFunc type,QObject *parent) :
    m_funcType(type),
    QObject(parent)
{
    m_logType=(m_funcType==FeedHolder)?Log_GrabHolder:Log_GrabPCB;
    m_threadBox = new ThreadBox(type,this);
    VisAppBus::subscibeEvent(this, "TrayReady");
    VisAppBus::subscibeEvent(this, "PipeLineTrayReady");
    VisAppBus::subscibeEvent(this, "SetClearFlag");
    VisAppBus::subscibeEvent(this, "HolderClearEnd");
}

ThreadFeed::~ThreadFeed()
{
    delete m_threadBox;
    stop();
}

void ThreadFeed::InitParam()
{
    m_errInfo="";
    m_trayTotalNum = 0;
    m_clearFlag = false;
    IOLevel level1 = VisMotorInstance->GetIoInput((m_funcType == FeedHolder) ? IN_HolderTrayTilt1:IN_PCBTrayTilt1);
    IOLevel level2 = VisMotorInstance->GetIoInput((m_funcType == FeedHolder) ? IN_HolderTrayTilt2:IN_PCBTrayTilt2);

    m_waitFeedTray = (level1 == IO_OFF);
    m_waitBlankTray = true;
    m_grabIndex = 0;
    m_flagStopBlankHolder = false;
    m_emptyPCBTray = 0;
    m_vecFeedModule.clear();
    m_threadBox->InitParam();

    start();
}

bool ThreadFeed::doTask()
{
    if (VisMotorInstance->IsEmgStop())return false;
    if (!Process()) {
        if (!m_errInfo.isEmpty())
            ShowSystemLog(Log_Error, m_errInfo);
        return false;
    }
    return true;
}

bool ThreadFeed::Process()
{
    //等待托盘
    if (m_waitFeedTray) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        return true;
    }
    //从料盘抓料
    int nRes = GrabTrayModule();
    if (nRes != 0) return false;
    if (!m_vecFeedModule.size())return true;
    //放置上料模组到测试位(PCB扫码在PlaceModule放置完成后执行,壳体扫码已屏蔽)
    nRes = PlaceModule();
    if (nRes != 0)return false;

    return true;
}

int ThreadFeed::GrabTrayModule()
{
    m_vecFeedModule.clear();
    int nRes=-1;
    ShowLog(m_logType, 0, Log_Info, QString(u8"开始抓取托盘模组%1").arg(m_grabIndex + 1));
    //检测托盘模组
    nRes = DetectTrayModule();
    if (nRes != 0) return nRes;
    //打开夹爪
    nRes = SetGripClose(false);
    if (nRes != 0)return nRes;
    //夹爪运动到托盘孔位
    nRes = MoveToTrayHole(m_grabIndex);
    if (nRes != 0)return nRes;
    //闭合夹爪
    nRes = SetGripClose(true);
    if (nRes != 0)return nRes;
    //移动到安全位
    nRes = VisMotorInstance->MovePositionAbs((m_funcType == FeedHolder) ? HolderGripSafe : PCBGripSafe);
    if (nRes != 0)return nRes;
    //检查夹爪模组,二次检查防止夹取后掉落
    bool exist=false;
    nRes = CheckGripModuleExist(exist);
    if (nRes != 0)return nRes;
    TrayFunc sendFunc = m_funcType;
    QString holeText = u8"空";
    // 抓取后物料取走，blankFlag=true，走灰色空仓逻辑
    VisAppBus::sendEvent("TrayHoleUpdate", sendFunc, true, false, m_grabIndex, holeText);
    m_grabIndex++;
    //检查是否托盘最后一个孔
    int totalHole = (m_funcType == FeedHolder) ? GlobalParam->recipeTray.feedTrayPosHolder.size() : GlobalParam->recipeTray.feedTrayPosPCB.size();
    if (m_grabIndex >= totalHole) {
        sigBlankTray(m_funcType);
    }
    if (!exist)return 0;
    m_vecFeedModule.push_back("");
    return 0;
}

int ThreadFeed::DetectTrayModule()
{
    return 0;
    RecipeTray& recipeTray = GlobalParam->recipeTray;
    QVector4D ptGrip = (m_funcType==FeedHolder)? recipeTray.feedTrayPosHolder.at(m_grabIndex): recipeTray.feedTrayPosPCB.at(m_grabIndex);
    QMap<QString, double> mapGroup;
    mapGroup[(m_funcType == FeedHolder) ? MotorHolderGantryX : MotorPCBGantryX] = ptGrip.x() + GlobalParam->hardwareParam.gripToCamX;
    mapGroup[(m_funcType == FeedHolder) ? MotorHolderGantryY : MotorPCBGantryY] = ptGrip.y() + GlobalParam->hardwareParam.gripToCamY;
    int nRes = VisMotorInstance->MoveAbsGroup(mapGroup);
    if (nRes != 0) return nRes;
    cv::Mat  img;
    if (!VisMotorDataInstance->m_flagOffline) {
        //		nRes = VisCameraTool::instance()->GrabImgFrame(0, PoseCam, img);
        //		if (nRes != 0) {
        //			m_errInfo = PoseCam + QString(u8"相机抓图失败:%1").arg(nRes);
        //			ShowLog(m_logType, 0, Log_Error, QString(u8"获取托盘码"));
        //			return nRes;
        //		}
    }
    else {

    }

    return 0;
}

int ThreadFeed::ScanCode()
{
    //移动到安全位
    int nRes = VisMotorInstance->MovePositionAbs((m_funcType == FeedHolder) ? HolderGripSafe : PCBGripSafe);
    if (nRes != 0)return nRes;

    //到扫码位
    VisMotorInstance->MovePositionAbs((m_funcType == FeedHolder) ? HolderScanCode : PCBScanCode);
    if (nRes != 0) return nRes;
    ShowLog(m_logType, 0, Log_Info, QString(u8"扫描产品条码"));
    QString scanName = (m_funcType == FeedHolder) ? ScanHolderCode : ScanPCBCode;
    QString barCode;
    VisAppBus::sendEvent("GetBarCode", scanName, barCode);
    m_vecFeedModule[0] = barCode;

    //PCB扫码后发送MES条码校验
    if (m_funcType == FeedPCB && !barCode.isEmpty()) {
        bool validate = false;
        int vRes = VisAppBus::sendEvent("MesValidateNumber", barCode, validate);
        ShowLog(m_logType, 0, vRes == 0 ? Log_Info : Log_Error,
            QString(u8"PCB条码校验%1:%2").arg(vRes == 0 ? u8"完成" : u8"失败").arg(validate ? u8"通过" : u8"不通过"));
    }

    return 0;
}

int ThreadFeed::PlaceModule()
{
    QMap<QString, double> moveGroup = VisMotorDataInstance->GetPosMap((m_funcType == FeedHolder) ? HolderPlace : PCBPlace);
    double moveZ = moveGroup[(m_funcType == FeedHolder) ? MotorHolderGantryZ : MotorPCBGantryZ];
    moveGroup.remove((m_funcType == FeedHolder) ? MotorHolderGantryZ : MotorPCBGantryZ);
    //执行放置点位xyr
    int nRes = VisMotorInstance->MoveAbsGroup(moveGroup);
    if (nRes != 0) return nRes;
    ShowLog(m_logType, 0, Log_Info, QString(u8"查询是否等待载具"));
    //检查是=是否处于等待托盘状态
    while (1) {
        if (!m_waitBlankTray) break;
        if(VisMotorInstance->IsEmgStop())return HardWareErr;
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    //托盘已流走(空壳体),不执行放置,回到Process循环等下一盘
    if (m_traySkip) {
        m_traySkip = false;
        sigBlankPipeLineTray(m_funcType, m_trayInfo);
        return 0;
    }
    ShowLog(m_logType, 0, Log_Info, QString(u8"开始放置到流水线托盘"));
    //壳体上料:伸出接驳台固定托盘,防止放置时移位
    if (m_funcType == FeedHolder) {
        VisMotorInstance->SetDoubleIoOutput(Out_TransferExend, IO_ON, Out_TransferBack, IO_OFF);
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
    //执行放置点位Z
    nRes = VisMotorInstance->MotorMoveAbs((m_funcType == FeedHolder) ? MotorHolderGantryZ : MotorPCBGantryZ, moveZ);
    if (nRes != 0) return nRes;
    //松开夹爪
    nRes = SetGripClose(false);
    if (nRes != 0) return nRes;
    //移动到安全位
    nRes = VisMotorInstance->MovePositionAbs((m_funcType == FeedHolder) ? HolderGripSafe : PCBGripSafe);
    if (nRes != 0)return nRes;
    //检查模组
    bool exist=false;
    nRes = CheckGripModuleExist(exist);
    if (nRes != 0)return nRes;
    if (GlobalParam->flagOffline || GlobalParam->emptyRun)
        exist = false;
    if(exist){
        m_errInfo = QString(u8"%1夹爪松开后仍检测到模组").arg((m_funcType == FeedHolder) ? u8"壳体" : u8"PCB");
        ShowLog(m_logType, 0, Log_Error, m_errInfo);
        VisAppBus::sendEvent("PopupErrInfo", m_errInfo);
        return HardWareErr;
    }
    ShowLog(m_logType, 0, Log_Info, QString(u8"放置完毕"));
    //壳体上料完毕:缩回接驳台,托盘流走
    if (m_funcType == FeedHolder) {
        VisMotorInstance->SetDoubleIoOutput(Out_TransferBack, IO_ON, Out_TransferExend, IO_OFF);
    }
    //PCB放置完成后扫码
    if (m_funcType == FeedPCB) {
        nRes = ScanCode();
        if (nRes != 0)return nRes;
    }
    //发送流走托盘请求
    m_trayInfo.empty = false;
    //壳体无二维码,扫码屏蔽,holderBarCode给假数据占位;PCB条码取扫码结果
    if (m_funcType == FeedHolder)
        m_trayInfo.holderBarCode = QString("Holder%1").arg(m_grabIndex);
    else {
        m_trayInfo.pcbBarCode = m_vecFeedModule[0];
        //扫码失败:条码为空→result=false,errInfo=扫码失败
        if (m_vecFeedModule[0].isEmpty()) {
            m_trayInfo.result = false;
            m_trayInfo.errInfo = u8"扫码失败";
            ShowLog(m_logType, 0, Log_Error, QString(u8"PCB扫码失败,托盘标记NG"));
        }
    }
    sigBlankPipeLineTray(m_funcType, m_trayInfo);
    return 0;
}

int ThreadFeed::MoveToTrayHole(int hole)
{
    QMap<QString, double> mapGroup;
    QVector4D pos = (m_funcType == FeedHolder) ? GlobalParam->recipeTray.feedTrayPosHolder.at(hole) : GlobalParam->recipeTray.feedTrayPosPCB.at(hole);
    mapGroup[(m_funcType == FeedHolder) ? MotorHolderGantryX : MotorPCBGantryX] = pos.x();
    mapGroup[(m_funcType == FeedHolder) ? MotorHolderGantryY : MotorPCBGantryY] = pos.y();
    mapGroup[(m_funcType == FeedHolder) ? MotorHolderGripR : MotorPCBGripR] = pos.w();
    int nRes=VisMotorInstance->MoveAbsGroup(mapGroup);
    if (nRes != 0) return nRes;
    nRes = VisMotorInstance->MotorMoveAbs((m_funcType == FeedHolder) ? MotorHolderGantryZ : MotorPCBGantryZ,pos.z());
    if (nRes != 0) return nRes;

    return 0;
}

int ThreadFeed::SetGripClose(bool close)
{
    int nRes = 0;
    if (close) {
        //闭合夹爪:固定到10(张开按伺服配方)
        nRes = VisMotorInstance->MotorMoveAbs((m_funcType == FeedHolder) ? MotorHolderGripX : MotorPCBGripX, 10);
        if (nRes != 0) return nRes;
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
    else {
        nRes = VisMotorInstance->MovePositionAbs((m_funcType == FeedHolder) ? HolderGripOpen : PCBGripOpen);
        if (nRes != 0) return nRes;
    }

    return 0;
}

int ThreadFeed::CheckGripModuleExist(bool& exist)
{
    if (GlobalParam->flagOffline|| GlobalParam->emptyRun) {
        exist = true;
        return 0;
    }

    curSportState state;
    if (!VisMotorInstance->ReadCurSportState((m_funcType == FeedHolder) ? MotorHolderGripX : MotorPCBGripX, state)) {
        m_errInfo = QString(u8"获取上料%1夹取状态失败").arg((m_funcType == FeedHolder) ? u8"壳体" : u8"PCB");
        ShowSystemLog(Log_Error, m_errInfo);
        VisAppBus::sendEvent("PopupErrInfo", m_errInfo);
        return HardWareErr;
    }
    exist = (state == SPAORTSTOPANDNOCATCH);

    return 0;
}

void ThreadFeed::sigBlankTray(TrayFunc type)
{
    ShowLog(m_logType, 0, Log_Info, QString(u8"请求退空托盘"));
    m_grabIndex = 0;
    m_waitFeedTray=true;
    VisAppBus::sendEventDirect("BlankTray",type);
}

void ThreadFeed::sigBlankPipeLineTray(TrayFunc type, TrayInfo info)
{
    m_waitBlankTray=true;
    VisAppBus::sendEventDirect("BlankPipeLineTray", type, info);
}

void ThreadFeed::sigHolderClearEnd()
{
    VisAppBus::sendEventDirect("HolderClearEnd");
}

int ThreadFeed::event_TrayReady(TrayFunc type)
{
    if (m_funcType != type)return 0;
    m_waitFeedTray = false;
    return 0;
}

int ThreadFeed::event_PipeLineTrayReady(TrayFunc type, TrayInfo info)
{
    if (m_funcType != type)return 0;
    m_trayTotalNum++;
    if (m_funcType == FeedHolder) {
        //前4个托盘不放壳体，或清料停止放壳体
        if (m_trayTotalNum < m_indexHolderTrayWork || m_flagStopBlankHolder) {
            sigBlankPipeLineTray(type, info);
            return 0;
        }
    }
    else if (m_funcType == FeedPCB) {
        //从第5个托盘开始，空盘空壳体直接退盘
        if (info.holderBarCode.isEmpty()&& m_trayTotalNum>= m_indexHolderTrayWork) {
            sigBlankPipeLineTray(type, info);
            return 0;
        }
        //清料模式后，流走4个有壳体无PCB托盘，告知上料壳体模块不再放壳体
        if (m_clearFlag) {
            m_emptyPCBTray++;
            if (m_emptyPCBTray == 4) {
                m_clearFlag = false;
                sigHolderClearEnd();
            }
            sigBlankPipeLineTray(type, info);
            return 0;
        }
    }

    m_trayInfo = info;
    m_waitBlankTray=false;
    return 0;
}

int ThreadFeed::event_HolderClearEnd()
{
    m_flagStopBlankHolder = true;
    return 0;
}

int ThreadFeed::event_SetClearFlag()
{
    if (m_funcType != FeedPCB)return 0;
    if (m_clearFlag)return 0;
    //当前上料PCB流线无载具
    if (m_waitBlankTray) {
        ShowLog(m_logType, 0, Log_Debug, QString(u8"请等上料PCB流线载具到位后再清料"));
        return 0;
    }
    m_waitBlankTray = true;
    m_clearFlag = true;
    m_emptyPCBTray = 1;
    sigBlankPipeLineTray(m_funcType, m_trayInfo);

    return 0;
}
