#include <QTime>
#include <QThread>
#include "WidgetScanCodeDebug.h"
#include "ui_WidgetScanCodeDebug.h"
#include "VisAppBus.h"
#include "ParamManager.h"
#include "VisUIParam.h"
#include "ScanCodeForm.h"
#include <QTextCodec>

#pragma execution_character_set("utf-8")

WidgetScanCodeDebug::WidgetScanCodeDebug(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WidgetScanCodeDebug)
{
    ui->setupUi(this);
    InitWidget();
}

WidgetScanCodeDebug::~WidgetScanCodeDebug()
{
    delete ui;
}

void WidgetScanCodeDebug::InitWidget()
{
    ScanCodeForm *widgetScan = new ScanCodeForm;
    widgetScan->SetName(ScanHolderCode);
    ui->gridLayout->addWidget(widgetScan,0,0);
    m_scanCodeFormList.append(widgetScan);

    ScanCodeForm *widgetScan2 = new ScanCodeForm;
    widgetScan2->SetName(ScanPCBCode);
    ui->gridLayout->addWidget(widgetScan2,1,0);
    m_scanCodeFormList.append(widgetScan2);
}


void WidgetScanCodeDebug::LoadUIParam()
{
    QString filename = GlobalParam->hardwareParam.fileName;
    QSettings *config = new QSettings(filename, QSettings::IniFormat);
    config->setIniCodec(QTextCodec::codecForName("UTF-8"));

    config->beginGroup(QString("ScanHolderCode"));
    VisUIParam::LoadIniToUI(config, m_scanCodeFormList.at(0), &GlobalParam->hardwareParam.trayCodeParam);
    config->endGroup();

    config->beginGroup(QString("ScanPCBCode"));
    VisUIParam::LoadIniToUI(config, m_scanCodeFormList.at(1), &GlobalParam->hardwareParam.pcbCodeParam);
    config->endGroup();
    delete config;
}

void WidgetScanCodeDebug::UpdateParamToUI()
{
     LoadUIParam();
}

void WidgetScanCodeDebug::SaveUIParam()
{
    QString filename = GlobalParam->hardwareParam.fileName;
    QSettings *config = new QSettings(filename, QSettings::IniFormat);
    config->setIniCodec(QTextCodec::codecForName("UTF-8"));

    config->beginGroup(QString("ScanHolderCode"));
    VisUIParam::SaveUIToIni(config, m_scanCodeFormList.at(0), &GlobalParam->hardwareParam.trayCodeParam);
    config->endGroup();

    config->beginGroup(QString("ScanPCBCode"));
    VisUIParam::SaveUIToIni(config, m_scanCodeFormList.at(1), &GlobalParam->hardwareParam.pcbCodeParam);
    config->endGroup();
    delete config;
}
