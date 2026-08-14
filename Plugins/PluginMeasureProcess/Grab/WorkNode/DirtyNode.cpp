#include "DirtyNode.h"
#include "VisAppBus.h"
#include "VisMotorManager.h"
#include "VisAppThreadPool.h"
#include "VisCameraTool.h"
#include "ParamManager.h"

EAD_Handle DirtyNode::s_detector = nullptr;
bool       DirtyNode::s_initialized = false;

DirtyNode::DirtyNode(ModuleInfo* item,int station)
{
    m_item = item;
    m_station = station;

    InitDetector();
}


int DirtyNode::InitDetector()
{	
    if (s_initialized) return 0;
    RecipeDirty& cfg = GlobalParam->recipeDirty;
    if (cfg.modelPath.isEmpty() || cfg.modelPathMtr.isEmpty()) {
        ShowSystemLog(Log_Error, QString(u8"脏污模型路径未配置,跳过初始化"));
        return -1;
    }
    s_detector = EAD_Create();
    if (!s_detector) {
        ShowSystemLog(Log_Error, QString(u8"EAD_Create 失败"));
        return -1;
    }
    EAD_Config config;
    config.model_path      = cfg.modelPath.toLocal8Bit().constData();
    config.threshold       = static_cast<float>(cfg.threshold);
    config.mask_threshold  = static_cast<float>(cfg.maskThreshold);
    config.enable_openvino = cfg.enableOpenvino;
    config.intra_threads   = cfg.intraThreads;
    int ret = EAD_Init(s_detector, &config);
    if (ret != EAD_OK) {
        ShowSystemLog(Log_Error, QString(u8"EAD_Init 失败:%1").arg(ret));
        EAD_Destroy(s_detector);
        s_detector = nullptr;
        return -1;
    }
    std::string mtrPath = cfg.modelPathMtr.toStdString();
    ret = InitModel(mtrPath);
    if (ret != 0) {
        ShowSystemLog(Log_Error, QString(u8"InitModel(MTR) 失败:%1").arg(ret));
        EAD_Destroy(s_detector);
        s_detector = nullptr;
        return -1;
    }
    s_initialized = true;
    ShowSystemLog(Log_Info, QString(u8"脏污检测器初始化成功"));
    return 0;
}

void DirtyNode::ReleaseDetector()
{
    if (s_detector) { EAD_Destroy(s_detector); s_detector = nullptr; }
    s_initialized = false;
}

int DirtyNode::event_TestDetect(int station)
{
    if (!s_initialized && InitDetector() != 0) {
        VisAppBus::sendEvent("ShowResultImage", station, QString(u8"脏污模型加载失败"));
        return -1;
    }
    cv::Mat img; QImage imgShow;
    int nRes = VisCameraTool::instance()->GrabImgFrame(station, DirtyDetectCam, img, &imgShow);
    if (nRes != 0) { VisAppBus::sendEvent("ShowResultImage", station, QString(u8"脏污相机抓图失败:%1").arg(nRes)); return -1; }
    cv::Mat processed; int ret = PreprocessImg(img, processed);
    if (ret != 0) { VisAppBus::sendEvent("ShowResultImage", station, QString(u8"预处理失败:%1").arg(ret)); return -1; }
    cv::Mat binary_mask; EAD_Result result;
    ret = EAD_DetectMat(s_detector, processed, binary_mask, &result);
    if (ret != EAD_OK) { VisAppBus::sendEvent("ShowResultImage", station, QString(u8"检测失败:%1").arg(ret)); return -1; }
    QString ngInfo = (result.is_ng == 0) ? "OK" : QString(u8"NG(%1)").arg(result.score);
    VisAppBus::sendEvent("ShowResultImage", station, ngInfo);
    if (result.is_ng != 0 && !binary_mask.empty()) { /* ...mask叠加... */ }
    else VisAppBus::sendEvent("ShowDirtyImg", station, imgShow);
    return 0;
}


int DirtyNode::Process()
{
    ShowLog(Log_Fixture, m_station, Log_Info, QString(u8"开始脏污检测%1").arg(m_station + 1));
    cv::Mat img;
    QImage imgShow;
    int nRes = VisCameraTool::instance()->GrabImgFrame(m_station, DirtyDetectCam, img, &imgShow);
    if (nRes != 0) {
        QString errInfo = QString(u8"脏污相机抓图失败:%1").arg(nRes);
        ShowLog(Log_Fixture, m_station, Log_Error, errInfo);
        //脏污NG:抓图失败标记result=false
        m_item->result = false;
        m_item->ngReason = u8"脏污NG";
        m_item->errorMsg = errInfo;
    }
    sigShowDirtyImg(m_station, imgShow);
    ShowLog(Log_Fixture, m_station, Log_Info, QString(u8"脏污检测结束%1").arg(m_station + 1));
    return 0;
}

void DirtyNode::sigShowDirtyImg(int station, QImage imgShow)
{
    VisAppBus::sendEvent("ShowDirtyImg", station, imgShow);
}
