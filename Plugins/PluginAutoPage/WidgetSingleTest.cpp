#include "WidgetSingleTest.h"
#include "ui_WidgetSingleTest.h"
#include "VisAppBus.h"
#include "ParamManager.h"

WidgetSingleTest::WidgetSingleTest(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WidgetSingleTest)
{
    ui->setupUi(this);
    VisAppBus::subscibeEvent(this, "UiAutoMode");
    VisAppBus::subscibeEvent(this, "LoginUserChange");
    InitSlot();
}

WidgetSingleTest::~WidgetSingleTest()
{
    delete ui;
}

void WidgetSingleTest::InitSlot()
{
    connect(ui->btnEndProduct, &QPushButton::clicked, this, [=]{
        //结束生产:触发清料,剩余托盘做完不放新盘
        VisAppBus::sendEvent("SetProductEnd", true);
        VisAppBus::sendEvent("SetClearFlag");
    });
}

int WidgetSingleTest::event_LoginUserChange()
{
    UserInfo& curUserInfo = GlobalParam->frameCore->curUserInfo;
    bool enable = (curUserInfo.authority == PARAMADMIN || curUserInfo.authority == SUPERADMIN);
    this->setEnabled(enable);
    return 0;
}

int WidgetSingleTest::event_UiAutoMode(bool flag)
{
    this->setEnabled(!flag);
    return 0;
}
