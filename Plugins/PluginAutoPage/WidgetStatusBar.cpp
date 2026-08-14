#include "WidgetStatusBar.h"
#include "ui_WidgetStatusBar.h"
#include "ParamManager.h"

WidgetStatusBar::WidgetStatusBar(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WidgetStatusBar)
{
    ui->setupUi(this);
    m_timer=new QTimer(this);
    m_timer->setInterval(500);
    connect(m_timer,&QTimer::timeout,this,&WidgetStatusBar::slotTimeout);
    m_timer->start();
}

WidgetStatusBar::~WidgetStatusBar()
{
    delete ui;
}

void WidgetStatusBar::slotTimeout()
{
    if(GlobalParam->frameCore==nullptr)return;
	QStringList listUserLevel;
	listUserLevel << u8"工程师" << u8"维护员" << u8"操作员" << u8"超级管理员";
	UserInfo& curUserInfo = GlobalParam->frameCore->curUserInfo;
	ui->label_user->setText(curUserInfo.userName + QString(u8"(%1)").arg(listUserLevel.at(curUserInfo.authority)));
    if (!GlobalParam->systemParam.shieldParam.mes)
    {
        ui->label_user_2->setText(u8"MES上传已开启");
        ui->label_user_2->setStyleSheet("background-color: rgb(0, 255, 0);");
    }
    else
    {
        ui->label_user_2->setText(u8"MES上传已关闭");
        ui->label_user_2->setStyleSheet("background-color: rgb(125, 125, 125);");
    }
}
