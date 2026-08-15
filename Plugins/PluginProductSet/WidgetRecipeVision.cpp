#include "WidgetRecipeVision.h"
#include "ui_WidgetRecipeVision.h"
#include <QMessageBox>
#include <QDateTime>
#include <QInputDialog>
#include <QDomDocument>
#include <QXmlStreamReader>
#include <QFile>
#include <QDebug>
#include "VisUIParam.h"
#include "VisCameraTool.h"
#include "VisAppThreadPool.h"
#include "VisMotorManager.h"
#include "AlgorithmModule.h"

WidgetRecipeVision::WidgetRecipeVision(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WidgetRecipeVision)
{
    ui->setupUi(this);

    // 相机列表和算法模块事件订阅
    QStringList cl = ReadCameraNameList();
    VisAppBus::postEvent_Topic("CamearList", "setCamearList", cl);
    VisAppBus::subscibeEvent_Topic("halconMatchDllPlatform", "GetCvMatCam", this);
    VisAppBus::subscibeEvent_Topic("halconMatchDllPlatform", "SetCamEx", this);
    VisAppBus::subscibeEvent_Topic("halconMatchDllPlatform", "SetLight", this);

    // 算法模块参数配置窗口挂载
    ui->verticalLayout->addWidget(AlgorithmCtr->GetConfigWidget());
}

WidgetRecipeVision::~WidgetRecipeVision()
{
    delete ui;
}

void WidgetRecipeVision::on_btnSave_clicked()
{
    QString recipeName = ui->lineEdit->text();
    if (recipeName.isEmpty()) {
        QMessageBox::information(this, u8"提示信息", u8"保存的配方名不能为空");
        return;
    }
    if (m_recipeVison.listRecipe.contains(recipeName)) {
        QMessageBox::information(this, u8"提示信息", u8"保存的配方名不能重复");
        return;
    }
    // 保存当前算法参数
    if (m_algorithmInited)
        VisAppBus::sendEvent_Topic("SaveAlgorithmCfg", "SaveParam");
    // 复制当前配方的 AlgorithmConfig 到新配方名
    QString srcPath = m_recipeVison.filepath + m_recipeVison.curRecipe + ".xml";
    QString dstPath = m_recipeVison.filepath + recipeName + ".xml";
    if (QFile::exists(srcPath))
        QFile::copy(srcPath, dstPath);
    // 更新配方列表
    ui->lineEdit->setText("");
    ui->comboBox_Recipe->addItem(recipeName);
    m_recipeVison.listRecipe.push_back(recipeName);
    GlobalParam->recipeVison.listRecipe = m_recipeVison.listRecipe;
    QString filename = m_recipeVison.filepath + "Vision.xml";
    GlobalParam->SaveRecipeList(filename, GlobalParam->recipeVison.listRecipe);
}

void WidgetRecipeVision::LoadUIParam()
{
    QString filename = m_recipeVison.filepath + "Vision.xml";
    GlobalParam->LoadRecipeList(filename, m_recipeVison.listRecipe, ui->comboBox_Recipe);
}

void WidgetRecipeVision::SaveUIParam()
{
    if (m_algorithmInited)
        VisAppBus::sendEvent_Topic("SaveAlgorithmCfg", "SaveParam");
    if (GlobalParam->recipeVison.curRecipe == m_recipeVison.curRecipe)
        GlobalParam->recipeVison = m_recipeVison;
}

void WidgetRecipeVision::UpdateParamToUI()
{
    if (m_algorithmInited)
        VisAppBus::sendEvent_Topic("RestoreAlgorithmCfg", "RestoreParam");
}

int WidgetRecipeVision::LoadRecipeFile()
{
    QString filename = GlobalParam->recipeVison.filepath + "Vision.xml";
    GlobalParam->LoadRecipeList(filename, GlobalParam->recipeVison.listRecipe, ui->comboBox_Recipe);
    //curRecipe为空或不在列表时,优先当前产品对应的配方
    QString prefer = GlobalParam->recipeProduct.GetCurProductRecipe("recipeVison");
    if (GlobalParam->recipeVison.curRecipe.isEmpty()
        || !GlobalParam->recipeVison.listRecipe.contains(GlobalParam->recipeVison.curRecipe)) {
        int idx = GlobalParam->recipeVison.listRecipe.indexOf(prefer);
        if (idx < 0) idx = 0;
        if (idx < GlobalParam->recipeVison.listRecipe.size())
            GlobalParam->recipeVison.curRecipe = GlobalParam->recipeVison.listRecipe.at(idx);
    }
    VisUIParam::QObjectCopy(&GlobalParam->recipeVison, &m_recipeVison);
    m_recipeVison = GlobalParam->recipeVison;
    ui->comboBox_Recipe->blockSignals(true);
    int index = m_recipeVison.listRecipe.indexOf(m_recipeVison.curRecipe);
    if (index < 0) index = m_recipeVison.listRecipe.indexOf(prefer);   //当前产品对应配方
    if (index < 0) index = 0;
    ui->comboBox_Recipe->setCurrentIndex(index);
    ui->comboBox_Recipe->blockSignals(false);

    // 初始化算法模块，加载当前配方的 AlgorithmConfig
    QString cfgPath;
    if (m_recipeVison.filepath.isEmpty())
        m_recipeVison.filepath = GlobalParam->recipeProduct.productPath + "recipeVison/";
    if (!m_recipeVison.curRecipe.isEmpty())
        cfgPath = m_recipeVison.filepath + m_recipeVison.curRecipe + ".xml";
    else
        cfgPath = m_recipeVison.filepath + "AlgorithmConfig.xml";
    int ret = AlgorithmCtr->Init(cfgPath, GlobalParam->recipeProduct.curProduct);
    m_algorithmInited = (ret == 0);

    // 算法模块初始化后重新发送相机列表
    {
        QStringList cl = ReadCameraNameList();
        VisAppBus::postEvent_Topic("CamearList", "setCamearList", cl);
    }
    // 发送光源名称列表
    {
        QStringList ll = ReadLightNameList();
        LoadLightChannelMap();
        VisAppBus::postEvent_Topic("LightMapping", "setLightMapping", ll);
    }

    if (GlobalParam->recipeVison.curRecipe == m_recipeVison.curRecipe)
        GlobalParam->recipeVison = m_recipeVison;

    return 0;
}

void WidgetRecipeVision::on_comboBox_Recipe_currentIndexChanged(const QString &arg1)
{
    if (m_algorithmInited)
        VisAppBus::sendEvent_Topic("SaveAlgorithmCfg", "SaveParam");

    m_recipeVison.curRecipe = arg1;

    // 加载新配方的 AlgorithmConfig
    QString cfgPath;
    if (!arg1.isEmpty())
        cfgPath = m_recipeVison.filepath + arg1 + ".xml";
    else
        cfgPath = m_recipeVison.filepath + "AlgorithmConfig.xml";
    int ret = AlgorithmCtr->Init(cfgPath, GlobalParam->recipeProduct.curProduct);
    m_algorithmInited = (ret == 0);
}

// 算法模块获取图像（签名对齐算法库：通过 cv::Mat& 直接回传图像）
int WidgetRecipeVision::event_GetCvMatCam(QString AlgorithmName, QString cameraName, cv::Mat &img)
{

    QStringList parts = cameraName.split('_');
    QString camName  = parts.value(0);
    int stationId    = parts.value(1).toInt();

    // 检查相机是否已打开，未打开则自动打开+出流
    if (!VisCameraTool::instance()->GetOpenState(stationId, camName))
    {
        int nRes = VisCameraTool::instance()->OpenCamera(stationId, camName);
        if (nRes != 0)
        {
            ShowSystemLog(Log_Error, QString(u8"相机[%1]打开失败:%2").arg(cameraName).arg(nRes));
            return -1;
        }
    }
    if (!VisCameraTool::instance()->GetStreamState(stationId, camName))
    {
        int nRes = VisCameraTool::instance()->StartStream(stationId, camName, false);
        if (nRes != 0)
        {
            ShowSystemLog(Log_Error, QString(u8"相机[%1]出流失败:%2").arg(cameraName).arg(nRes));
            return -1;
        }
    }

    return VisCameraTool::instance()->GrabImgFrame(stationId, camName, img);
}

// 算法模块设置相机曝光
int WidgetRecipeVision::event_SetCamEx(QString AlgorithmName, QString cameraName, int CamEx)
{
    QStringList parts = cameraName.split('_');
    QString camName  = parts.value(0);
    int stationId    = parts.value(1).toInt();
    VisCameraTool::instance()->SetCamParam(stationId, camName, VisCameraTool::ShutterTime, CamEx);
    return 0;
}

// 算法模块设置光源亮度
int WidgetRecipeVision::event_SetLight(QString AlgorithmName, QString LightName, bool bSwitch, int red, int green, int blue, int Lum)
{
    auto& param = GlobalParam->hardwareParam.lightParam;
    QString portName = param.StrlightPort;
    VisAppBus::sendEvent("InitLightPort", portName, param.StrlightBaud.toInt());
    int channel = m_lightChannelMap.value(LightName, 1);
    if (bSwitch) {
        int brightness = qBound(0, Lum, 255);
        VisAppBus::sendEvent("SetLightValue", portName, channel, brightness);
    } else {
        VisAppBus::sendEvent("TurnOffLight", portName, channel);
    }
    return 0;
}

QStringList WidgetRecipeVision::ReadCameraNameList()
{
    QStringList camNameList;
    QString path = QCoreApplication::applicationDirPath() + "/Config/Camera.xml";
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
    {
        qWarning() << "打开相机配置文件失败:" << path;
        return camNameList;
    }

    QXmlStreamReader xml(&file);
    while (!xml.atEnd() && !xml.hasError())
    {
        QXmlStreamReader::TokenType token = xml.readNext();
        if (token == QXmlStreamReader::StartElement)
        {
            if (xml.name() == "Cam")
            {
                QString camName = xml.attributes().value("name").toString();
                // 读 Cam 内部的 Item
                while (!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == "Cam"))
                {
                    xml.readNext();
                    if (xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == "Item")
                    {
                        int idx = xml.attributes().value("index").toInt();
                        QString showName = QString("%1_%2").arg(camName).arg(idx);
                        camNameList << showName;
                    }
                }
            }
        }
    }
    file.close();
    return camNameList;
}

QStringList WidgetRecipeVision::ReadLightNameList()
{
    QStringList lightNameList;
    QString path = QCoreApplication::applicationDirPath() + "/Config/Light.xml";
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) return lightNameList;

    QXmlStreamReader xml(&file);
    while (!xml.atEnd() && !xml.hasError()) {
        if (xml.readNext() == QXmlStreamReader::StartElement && xml.name() == "Light")
            lightNameList << xml.attributes().value("name").toString();
    }
    file.close();
    return lightNameList;
}

void WidgetRecipeVision::LoadLightChannelMap()
{
    m_lightChannelMap.clear();
    QString path = QCoreApplication::applicationDirPath() + "/Config/Light.xml";
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) return;

    QXmlStreamReader xml(&file);
    while (!xml.atEnd() && !xml.hasError()) {
        if (xml.readNext() == QXmlStreamReader::StartElement && xml.name() == "Light") {
            QString name = xml.attributes().value("name").toString();
            int channel = xml.attributes().value("channel").toInt();
            m_lightChannelMap[name] = channel;
        }
    }
    file.close();
}




