#include "WidgetProductSet.h"
#include "ui_WidgetProductSet.h"
#include "WidgetMatrixCfg.h"
#include "WidgetRecipePlatform.h"
#include "WidgetRecipeMotor.h"
#include "WidgetRecipeVision.h"
#include "WidgetRecipeTray.h"
#include "WidgetRecipeFilmTear.h"
#include "WidgetRecipeGrip.h"
#include "WidgetRecipeDirty.h"

#include <QMessageBox>

WidgetProductSet::WidgetProductSet(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WidgetProductSet)
{
    ui->setupUi(this);
    ui->widgetLeftBar->setObjectName("leftbar");
    ui->stackedWidget->setObjectName("stackWidget");
    WidgetMatrixCfg* widgetMatrix = new WidgetMatrixCfg(ui->stackedWidget);
    WidgetRecipePlatform* widgetPlatform = new WidgetRecipePlatform(ui->stackedWidget);
    WidgetRecipeMotor*widgetMotor=new WidgetRecipeMotor(ui->stackedWidget);
	WidgetRecipeVision* widgetVision = new WidgetRecipeVision(ui->stackedWidget);
    WidgetRecipeFilmTear*widgetFilmTear=new WidgetRecipeFilmTear(ui->stackedWidget);
    WidgetRecipeTray*widgetTray=new WidgetRecipeTray(ui->stackedWidget);
    WidgetRecipeGrip*widgetGrip=new WidgetRecipeGrip(ui->stackedWidget);
    WidgetRecipeDirty*widgetDirty=new WidgetRecipeDirty(ui->stackedWidget);
    ui->stackedWidget->addWidget(widgetMatrix);
    ui->stackedWidget->addWidget(widgetPlatform);
    ui->stackedWidget->addWidget(widgetMotor);
    ui->stackedWidget->addWidget(widgetTray);
    ui->stackedWidget->addWidget(widgetVision);
    ui->stackedWidget->addWidget(widgetFilmTear);
    ui->stackedWidget->addWidget(widgetGrip);
    ui->stackedWidget->addWidget(widgetDirty);

    connect(widgetMatrix, &WidgetMatrixCfg::sigAddPlatfrom, widgetPlatform, &WidgetRecipePlatform::slotAddPlatfrom);
    connect(GlobalParam->frameCore, &QVisF_Interface::sig_OutputInfo, this, &WidgetProductSet::slot_InputInfo);
    VisAppBus::subscibeEvent(this, "LoginUserChange");
}

WidgetProductSet::~WidgetProductSet()
{
    delete ui;
}

void WidgetProductSet::LoadUIParam()
{
	/* for (int i = 0; i < ui->stackedWidget->count(); i++) {
		 QMetaObject::invokeMethod(ui->stackedWidget->widget(i), "LoadUIParam", Qt::DirectConnection);
	 }*/
	for (int i = 0; i < ui->stackedWidget->count(); i++) {
		QMetaObject::invokeMethod(ui->stackedWidget->widget(i), "LoadUIParam", Qt::DirectConnection);
	}
}

void WidgetProductSet::UpdateUIParam()
{
    for (int i = 0; i < ui->stackedWidget->count(); i++) {
        QMetaObject::invokeMethod(ui->stackedWidget->widget(i), "UpdateParamToUI", Qt::DirectConnection);
    }
}

void WidgetProductSet::on_btnUpdateUI_clicked()
{
    if (QMessageBox::question(nullptr, u8"询问", u8"是否刷新当前页面参数") == QMessageBox::No)
        return ;
    QMetaObject::invokeMethod(ui->stackedWidget->currentWidget(), "UpdateParamToUI", Qt::DirectConnection);
}

void WidgetProductSet::on_btnSaveParam_clicked()
{
    if (QMessageBox::question(nullptr, u8"询问", u8"是否保存当前页面参数" ) == QMessageBox::No)
        return ;
	QMetaObject::invokeMethod(ui->stackedWidget->currentWidget(), "SaveUIParam", Qt::DirectConnection);
}

void WidgetProductSet::slot_InputInfo(tagOutputInfo& inputInfo)
{
    if (inputInfo._type == INFT_ProductChange) {
		int nRes = 0;
		for (int i = 0; i < ui->stackedWidget->count(); i++) {
			QMetaObject::invokeMethod(ui->stackedWidget->widget(i), "LoadRecipeFile", Qt::DirectConnection, Q_RETURN_ARG(int, nRes));
			if (nRes != 0)return;
		}
    }
}

int WidgetProductSet::event_LoginUserChange()
{
	UserInfo& curUserInfo = GlobalParam->frameCore->curUserInfo;
	bool enable = (curUserInfo.authority == SUPERADMIN || curUserInfo.authority == PARAMADMIN);
	ui->widget_Param->setEnabled(enable);
	for (int i = 0; i < ui->stackedWidget->count(); i++) {
		WidgetRecipeMotor* widgetMotor = qobject_cast<WidgetRecipeMotor*>(ui->stackedWidget->widget(i));
		if (widgetMotor != nullptr && curUserInfo.authority == ENGINEERADMIN) {
			widgetMotor->setEnabled(true);
		}
		else
			ui->stackedWidget->widget(i)->setEnabled(enable);
	}

	return 0;
}


