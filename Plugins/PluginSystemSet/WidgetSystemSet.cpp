#include "WidgetSystemSet.h"
#include "ui_WidgetSystemSet.h"
#include "WidgetGrip.h"
#include "WidgetShield.h"
#include "WidgetBox.h"
#include <QMessageBox>

WidgetSystemSet::WidgetSystemSet(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WidgetSystemSet)
{
    ui->setupUi(this);
    ui->widgetLeftBar->setObjectName("leftbar");
    ui->stackedWidget->setObjectName("stackWidget");
    WidgetShield* widgetShield = new WidgetShield(ui->stackedWidget);
    WidgetGrip*widgetGrip=new WidgetGrip(ui->stackedWidget);
    WidgetBox*widgetBox=new WidgetBox(ui->stackedWidget);
    ui->stackedWidget->addWidget(widgetShield);
    ui->stackedWidget->addWidget(widgetGrip);
    ui->stackedWidget->addWidget(widgetBox);
    //左侧导航切换页面
    connect(ui->btnShiled_1, &QToolButton::clicked, this, [this]{ ui->stackedWidget->setCurrentIndex(0); });
    connect(ui->btnGrip_2, &QToolButton::clicked, this, [this]{ ui->stackedWidget->setCurrentIndex(1); });
    connect(ui->btnBox_3, &QToolButton::clicked, this, [this]{ ui->stackedWidget->setCurrentIndex(2); });
    VisAppBus::subscibeEvent(this, "LoginUserChange");
}

WidgetSystemSet::~WidgetSystemSet()
{
    delete ui;
}

void WidgetSystemSet::LoadUIParam()
{
	for (int i = 0; i < ui->stackedWidget->count(); i++) {
		QMetaObject::invokeMethod(ui->stackedWidget->widget(i), "LoadUIParam", Qt::DirectConnection);
	}
}

void WidgetSystemSet::UpdateUIParam()
{
	for (int i = 0; i < ui->stackedWidget->count(); i++) {
		QMetaObject::invokeMethod(ui->stackedWidget->widget(i), "UpdateParamToUI", Qt::DirectConnection);
	}
}

int WidgetSystemSet::event_LoginUserChange()
{
	UserInfo& curUserInfo = GlobalParam->frameCore->curUserInfo;
	bool enable = (curUserInfo.authority != OPERATOR && curUserInfo.authority != ENGINEERADMIN);
	ui->widget_Param->setEnabled(enable);
	ui->stackedWidget->setEnabled(enable);
	return 0;
}


void WidgetSystemSet::on_btnUpdateUI_clicked()
{
    if (QMessageBox::question(nullptr, u8"询问", u8"是否刷新当前页面参数") == QMessageBox::No)
        return ;
	QMetaObject::invokeMethod(ui->stackedWidget->currentWidget(), "UpdateParamToUI", Qt::DirectConnection);
}

void WidgetSystemSet::on_btnSaveParam_clicked()
{
    if (QMessageBox::question(nullptr, u8"询问", u8"是否保存当前页面参数" ) == QMessageBox::No)
        return ;
	QMetaObject::invokeMethod(ui->stackedWidget->currentWidget(), "SaveUIParam", Qt::DirectConnection);
}


