#include "WidgetManualDebug.h"
#include "ui_WidgetManualDebug.h"
#include <QStyle>
#include <QMessageBox>
#include "VisAppBus.h"
#include "VisMotorTool.h"
#include "VisCameraTool.h"
#include "ScanCode/WidgetScanCodeDebug.h"
#include "WidgetLightCtl.h"
#include "WidgetManualCtl.h"
#include "ScanCode/WidgetTrayScanDebug.h"
#include "WidgetPressDispDebug.h"

using namespace VisMotorToolSpace;

WidgetManualDebug::WidgetManualDebug(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WidgetManualDebug)
{
    ui->setupUi(this);
    ui->widgetLeftBar->setObjectName("leftbar");
    ui->stackedWidget->setObjectName("stackWidget");

    //初始化IO窗口
    QWidget* widgetIO = VisMotorToolIns->GetWidget(VisMotorTool::F_IoMonitor);
    widgetIO->setObjectName("IoMonitor");
    widgetIO->setStyleSheet("QTabBar::tab { height: 30px;width: 80px; }\nQTabWidget#tabWidget::tab-bar { \n	alignment: center; \n} ");

    WidgetScanCodeDebug *widgetScanCode = new WidgetScanCodeDebug(ui->stackedWidget);
    WidgetTrayScanDebug *TrayScan = new WidgetTrayScanDebug(ui->stackedWidget);
    WidgetLightCtl*widgetLight = new WidgetLightCtl(ui->stackedWidget);
    WidgetManualCtl*widgetManu = new WidgetManualCtl(ui->stackedWidget);
    WidgetPressDispDebug *widgetPressDisp = new WidgetPressDispDebug(ui->stackedWidget);
    ui->stackedWidget->addWidget(widgetIO);
    ui->stackedWidget->addWidget(widgetScanCode);
    ui->stackedWidget->addWidget(widgetLight);
    ui->stackedWidget->addWidget(widgetManu);
    ui->stackedWidget->addWidget(TrayScan);
    ui->stackedWidget->addWidget(widgetPressDisp);
    VisAppBus::subscibeEvent(this, "LoginUserChange");
}

WidgetManualDebug::~WidgetManualDebug()
{
    delete ui;
}

void WidgetManualDebug::AddLog(QString msg, LogLevel level)
{
    ui->widget_Log->addLog(msg, level);
}

void WidgetManualDebug::LoadUIParam()
{
	for (int i = 0; i < ui->stackedWidget->count(); i++) {
		QMetaObject::invokeMethod(ui->stackedWidget->widget(i), "LoadUIParam", Qt::DirectConnection);
	}
}

int WidgetManualDebug::event_LoginUserChange()
{
	UserInfo& curUserInfo = GlobalParam->frameCore->curUserInfo;
	bool enable = (curUserInfo.authority != OPERATOR);
	ui->widget_Param->setEnabled(enable);
	ui->stackedWidget->setEnabled(enable);
	return 0;
}

void WidgetManualDebug::on_btnUpdateUI_clicked()
{
	if (QMessageBox::question(nullptr, u8"询问", u8"是否刷新当前页面参数") == QMessageBox::No)
		return;
	QMetaObject::invokeMethod(ui->stackedWidget->currentWidget(), "UpdateParamToUI", Qt::DirectConnection);
}

void WidgetManualDebug::on_btnSaveParam_clicked()
{
	if (QMessageBox::question(nullptr, u8"询问", u8"是否保存当前页面参数") == QMessageBox::No)
		return;
	QMetaObject::invokeMethod(ui->stackedWidget->currentWidget(), "SaveUIParam", Qt::DirectConnection);
}
