#include <QTime>
#include <QThread>
#include "ScanCodeForm.h"
#include "ui_ScanCodeForm.h"
#include "VisAppBus.h"
#include "ParamManager.h"
#include "VisUIParam.h"
#pragma execution_character_set("utf-8")

ScanCodeForm::ScanCodeForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ScanCodeForm)
{
    ui->setupUi(this);
    setMaximumWidth(550);
}

ScanCodeForm::~ScanCodeForm()
{
    delete ui;
}

void ScanCodeForm::SetName(QString chName)
{
    ui->groupBox->setTitle(chName);
    m_chName = chName;
}

void ScanCodeForm::on_PushButton_Connect_clicked()
{
    VisAppBus::sendEvent("ScanCodeConnect",m_chName);
}

void ScanCodeForm::on_PushButton_DisConnect_clicked()
{
    VisAppBus::sendEvent("DisScanCodeConnect",m_chName);
}

void ScanCodeForm::on_PushButton_ScanCode_clicked()
{
    QString barCode;
    VisAppBus::sendEvent("GetBarCode",m_chName,barCode);
    ui->LineEditCode->setText(barCode);
}
