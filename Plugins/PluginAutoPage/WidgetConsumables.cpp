#include "WidgetConsumables.h"
#include "ui_WidgetConsumables.h"
#include "ParamManager.h"
#include <QPainter>

WidgetConsumables::WidgetConsumables(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WidgetConsumables)
{
    ui->setupUi(this);

    VisAppBus::subscibeEvent(this, "ShowConsumables");
    VisAppBus::subscibeEvent(this, "CalibComplete");
    Init();
}

WidgetConsumables::~WidgetConsumables()
{
    delete ui;
}

int WidgetConsumables::event_ShowConsumables()
{
    QStringList listName;
    for(int i=0;i<GlobalParam->recipeProduct.listPlatformMatrix.size();i++)
        listName<<GlobalParam->recipeProduct.listPlatformMatrix.at(i).productShow;
    m_wearingPartsList.SetMoudleName(listName);
    m_wearingPartsList.close();
    m_wearingPartsList.showNormal();
    return 0;
}

int WidgetConsumables::event_CalibComplete(QVector<ModuleInfo> &info)
{
    m_wearingPartsList.Overhaul();                  //公共参数累加
    m_wearingPartsList.Overhaul(0, "non");  //工位不绑定机种累加
    //工位绑定机种累加
    m_wearingPartsList.Overhaul(0, GlobalParam->recipeProduct.curProduct);
    m_wearingPartsListUser->UpdateList(GlobalParam->recipeProduct.curProduct);
    return 0;
}

void WidgetConsumables::Init()
{
    m_wearingPartsListUser = new WearingPartsListUser(this);
    m_wearingPartsList.setAttribute(Qt::WA_QuitOnClose, false);
    ui->stackedWidgetWearingPartsList->addWidget(m_wearingPartsListUser);
    m_wearingPartsListUser->UpdateList("", true);

    connect(&m_wearingPartsList, &WearingPartsList::operationLog, this, [this](QString text){
        m_wearingPartsListUser->UpdateList(GlobalParam->recipeProduct.curProduct);
    });
}
