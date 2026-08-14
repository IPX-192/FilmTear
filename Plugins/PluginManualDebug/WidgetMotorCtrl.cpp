#include "WidgetMotorCtrl.h"
#include "ui_WidgetMotorCtrl.h"
#include "VisMotorTool.h"
#include "VisMotorToolData.h"
#include "VisMotorManager.h"
#include "ParamManager.h"

using namespace VisMotorToolSpace;

SINGLETON_IMPL(WidgetMotorCtrl)
WidgetMotorCtrl::WidgetMotorCtrl(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WidgetMotorCtrl)
{
    ui->setupUi(this);
    this->setWindowTitle(u8"电机调试");
    setAttribute(Qt::WA_QuitOnClose, false);
}

WidgetMotorCtrl::~WidgetMotorCtrl()
{
    delete ui;
}

void WidgetMotorCtrl::InitMotor()
{
	QString appPath = QCoreApplication::applicationDirPath();
	VisMotorToolIns->SetMotorFile(appPath + "/Config/MotorParam.xml");
	VisMotorToolIns->SetPointFile(appPath + "/Config/pos.xml");
    VisMotorDataInstance->m_flagOffline = GlobalParam->flagOffline;
    bool  bRet = VisMotorInstance->InitMotor(Motor_LTDMCBusIp, false);
    ShowSystemLog(bRet ? Log_Info : Log_Error, QString(u8"控制卡初始化%1！").arg(bRet ? u8"成功" : u8"失败"));
	if (!VisMotorInstance->ConnectGrip("COM9"))
		ShowSystemLog(Log_Error, QString(u8"电爪COM9初始化失败"));
    if (bRet) {
        VisMotorToolIns->StartIoMonitor();
        VisMotorToolIns->StartAxisMonitor();
    }
	
}

void WidgetMotorCtrl::showEvent(QShowEvent *event)
{
    if (!m_layout)
    {
        m_layout = new QVBoxLayout(this);
        m_widgetMotor = VisMotorToolIns->GetWidget(VisMotorTool::F_MotorDebugForm);
        m_widgetMotor->setMinimumHeight(550);
        m_layout->addWidget(m_widgetMotor);
        m_widgetIoDebug = VisMotorToolIns->GetWidget(VisMotorTool::F_IoDebugForm);
        m_widgetIoDebug->setMaximumHeight(200);
        m_layout->addWidget(m_widgetIoDebug);
        this->setLayout(m_layout);
    }
    QWidget::showEvent(event);
}

void WidgetMotorCtrl::closeEvent(QCloseEvent *event)
{
    if (m_layout)
    {
        delete m_layout;
        m_layout = nullptr;
    }

    VisAppBus::postEvent("MotorUiAdd");
    QWidget::closeEvent(event);
}
