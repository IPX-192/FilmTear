#include "WidgetAutoPage.h"
#include "ui_WidgetAutoPage.h"
#include <QApplication>
#include "ParamManager.h"
#include "WidgetLogAll.h"
#include "ProductForm/WidgetProdutData.h"
#include "WidgetTrayState.h"
#include "WidgetConsumables.h"
#include "WidgetCalibImage.h"
#include "WidgetVision.h"
#include "WidgetProductChange.h"
#include "WidgetFlowState.h"
#include "Product/WidgetProductTotal.h"

WidgetAutoPage::WidgetAutoPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WidgetAutoPage)
{
    ui->setupUi(this);
    ui->widgetLeftBar->setObjectName("leftbar");
    ui->stackedWidget->setObjectName("stackWidget");
    WidgetProductTotal*widgetProductTotal=new WidgetProductTotal(2,GlobalParam->frameCore->configDirPath,ui->stackedWidget);
    WidgetProdutData*widgetProdutData=new WidgetProdutData(ui->stackedWidget);
    WidgetCalibImage *widgetCalibImage = new WidgetCalibImage(ui->stackedWidget);
    WidgetVision *widgetVision = new WidgetVision(ui->stackedWidget);
    WidgetTrayState* widgetTrayState = new WidgetTrayState(ui->stackedWidget);
    WidgetFlowState* widgetFlowState = new WidgetFlowState(ui->stackedWidget);
    m_widgetLogAll = new WidgetLogAll(ui->stackedWidget);
    WidgetConsumables* widgetConsumables = new WidgetConsumables(ui->stackedWidget);
    //WidgetYield *widgetYield = new WidgetYield(ui->stackedWidget);
    WidgetProductChange *widgetProductChange = new WidgetProductChange(ui->stackedWidget);
    ui->stackedWidget->addWidget(widgetProductTotal);
    ui->stackedWidget->addWidget(widgetProdutData);
    ui->stackedWidget->addWidget(widgetCalibImage);
    ui->stackedWidget->addWidget(widgetVision);
    ui->stackedWidget->addWidget(widgetTrayState);
    ui->stackedWidget->addWidget(widgetFlowState);
    ui->stackedWidget->addWidget(m_widgetLogAll);
    ui->stackedWidget->addWidget(widgetConsumables);
    ui->stackedWidget->addWidget(widgetProductChange);
    ui->tbtnCalibImg_3->setVisible(false);
    ui->tbtnVision_4->setVisible(false);
	connect(ui->stackedWidget, &QStackedWidget::currentChanged, [=](int index) {
		//在标定图像页不显示
		//ui->widget_2->setVisible(index != 2);
		});
}

WidgetAutoPage::~WidgetAutoPage()
{
    delete ui;
}

void WidgetAutoPage::InitLog(QList<PluginLogInfo>& listPluginLog)
{
	PluginLogInfo pluginLog;
	pluginLog.type = SystemLog;
	pluginLog.index = 0;
	pluginLog._pLog = std::bind(&WidgetLog::addLog, ui->widget, std::placeholders::_1, std::placeholders::_2);
    listPluginLog.append(pluginLog);

    m_widgetLogAll->InitLog(listPluginLog);
}

