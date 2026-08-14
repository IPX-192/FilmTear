#include "WidgetTrayScanDebug.h"
#include "ui_WidgetTrayScanDebug.h"
#include "TrayRfidForm.h"
#include "ParamManager.h"
#include "VisUIParam.h"
#include <QSettings>
#include <QTextCodec>
#pragma execution_character_set("utf-8")

WidgetTrayScanDebug::WidgetTrayScanDebug(QWidget *parent)
    : QWidget(parent), ui(new Ui::WidgetTrayScanDebug)
{
    ui->setupUi(this);
    m_trayRfidForm = new TrayRfidForm(this);
    ui->gridLayout->addWidget(m_trayRfidForm);
}

WidgetTrayScanDebug::~WidgetTrayScanDebug()
{
    delete ui;
}

void WidgetTrayScanDebug::LoadUIParam()
{
    QString filename = GlobalParam->hardwareParam.fileName;
    QSettings *config = new QSettings(filename, QSettings::IniFormat);
    config->setIniCodec(QTextCodec::codecForName("UTF-8"));

    config->beginGroup(QString("TrayRfidParam"));
    VisUIParam::LoadIniToUI(config, m_trayRfidForm->GetUiWidget(), &GlobalParam->hardwareParam.trayRfidDebugParam);
    config->endGroup();

    delete config;
}

void WidgetTrayScanDebug::UpdateParamToUI()
{
    LoadUIParam();
}

void WidgetTrayScanDebug::SaveUIParam()
{
    QString filename = GlobalParam->hardwareParam.fileName;
    QSettings *config = new QSettings(filename, QSettings::IniFormat);
    config->setIniCodec(QTextCodec::codecForName("UTF-8"));

    config->beginGroup(QString("TrayRfidParam"));
    VisUIParam::SaveUIToIni(config, m_trayRfidForm->GetUiWidget(), &GlobalParam->hardwareParam.trayRfidDebugParam);
    config->endGroup();

    delete config;
}
