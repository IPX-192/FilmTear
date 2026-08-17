#include "WidgetBtnPanel.h"
#include "ui_WidgetBtnPanel.h"
#include <QTime>
#include <QInputDialog>
#include <QMessageBox>
#include "../Common/SwitchButton.h"
#include "VisAppBus.h"
#include "ParamManager.h"
#include "../Common/NonBlockingMsgBox.h"

WidgetBtnPanel::WidgetBtnPanel(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WidgetBtnPanel)
{
    ui->setupUi(this);
    m_autoModel = new SwitchButton();
    m_autoModel->SetTitle(QString::fromLocal8Bit("自动模式"));
    ui->frameAutoModel->layout()->addWidget(m_autoModel);
    connect(m_autoModel, &SwitchButton::CheckedChanged, this, &WidgetBtnPanel::slotAutoModel,Qt::QueuedConnection);
    connect(ui->btnClear, &QPushButton::clicked, this, [=]
    {
//        ui->btnReset->setEnabled(false);
//        m_autoModel->setEnabled(false);
//        VisAppBus::sendEvent("ClearModule");
//        ui->btnReset->setEnabled(true);
//        m_autoModel->setEnabled(true);
        VisAppBus::sendEvent("SetBackFlowReady", true, QString());   //手动放盘后通知回流托盘就绪(无托盘码)
    });
    VisAppBus::subscibeEvent(this, "SetMachineStatus");
}

WidgetBtnPanel::~WidgetBtnPanel()
{
    delete ui;
}

void WidgetBtnPanel::SetState(RunState state)
{
	QList<QPushButton*> tbtns = this->findChildren<QPushButton*>();
	for (auto btn : tbtns) {
        if (state == IdleState || state == ProduceState) {
            btn->setEnabled(true);
        }	
        else {
            m_autoModel->setEnabled(false);
            btn->setEnabled(false);
        }	
	}
	if (state == IdleState) {
		m_autoModel->setEnabled(true);
        m_autoModel->ChangeState(false);
        ChangeExeStatus(QString(u8"空闲"), QColor(0, 128, 0),true);
	}
	else if (state == ResetState) {
		ui->btnEstop->setEnabled(true);
        m_autoModel->setEnabled(false);
        ChangeExeStatus(QString(u8"复位"), QColor(255, 0, 0), false);
	}
	else if(state == ProduceState) {
		m_autoModel->setEnabled(false);
        ui->btnSpotCheck->setEnabled(false);
        ui->btnClear->setEnabled(true);
		ui->btnReset->setEnabled(false);
        ui->btnAutoRestore->setEnabled(false);
        ChangeExeStatus(QString(u8"自动"), QColor(0, 128, 0),false);
	}
}

void WidgetBtnPanel::slotAutoModel(bool checked)
{
    if (checked)
    {
        if (!m_autoModel->isEnabled())return;
        VisAppBus::sendEvent("InitCalibTabelTitle");
        SetState(ProduceState);
		VisAppBus::sendEvent("UiAutoMode", true);
        int nRes = VisAppBus::sendEvent("AutoStart");
        if (nRes != 0) {
            ShowSystemLog(Log_Error, QString(u8"进入生产模式失败"));
            SetState(IdleState);
			VisAppBus::sendEvent("UiAutoMode", false);
            return;
        } 
        GlobalParam->autoRunning = true;
    }
    else
    {
        VisAppBus::sendEvent("UiAutoMode", false);
        GlobalParam->autoRunning = false;
    }
}

void WidgetBtnPanel::on_btnReset_clicked()
{
    if (!ui->btnReset->isEnabled())return;
	VisAppBus::sendEvent("UiAutoMode", true);
	SetState(ResetState);
	int nRes = VisAppBus::sendEvent("AutoReset");
    SetState(IdleState);
	VisAppBus::sendEvent("UiAutoMode", false);
}

void WidgetBtnPanel::on_btnAutoPause_clicked()
{
    VisAppBus::sendEvent("AutoPause");
    ui->btnAutoPause->setEnabled(false);
    ui->btnAutoRestore->setEnabled(true);
}

void WidgetBtnPanel::on_btnAutoRestore_clicked()
{
    VisAppBus::sendEvent("AutoResume");
	ui->btnAutoPause->setEnabled(true);
    ui->btnAutoRestore->setEnabled(false);
}

void WidgetBtnPanel::on_btnAlarmCleared_clicked()
{
    VisAppBus::sendEvent("ClearWarning");
}

void WidgetBtnPanel::on_btnEstop_clicked()
{
    ShowSystemLog(Log_Error, QString(u8"界面按钮触发急停"));
    VisAppBus::sendEvent("AutoEmg");
    SetState(IdleState);
}

int WidgetBtnPanel::event_SetMachineStatus(MachineRunStatus status)
{
    if (status == AutoRunning) {
        slotAutoModel(true);
        tagOutputInfo outInfo;
        outInfo._type = INFT_MACHINE_Auto;
        emit GlobalParam->frameCore->sig_OutputInfo(outInfo);
    }
    else if (status == Homed) {
        on_btnReset_clicked();
    }
    else if (status == Pause) {
        if (ui->btnAutoPause->isEnabled())
            on_btnAutoPause_clicked();
        else
            on_btnAutoRestore_clicked();
    }
    return 0;
}
