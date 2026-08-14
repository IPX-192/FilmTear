#include "WidgetPressDispDebug.h"
#include "ParamManager.h"
#include "VisUIParam.h"
#include "VisAppBus.h"
#include <QSettings>
#include <QTextCodec>
#include <QSerialPortInfo>
#include <QVariant>
#include <QtCore/QMetaType>
#include <QTime>
#include <QDebug>
#include <QMetaProperty>
#pragma execution_character_set("utf-8")

WidgetPressDispDebug::WidgetPressDispDebug(QWidget *parent)
    : QWidget(parent)
{
    this->setMaximumWidth(920);
    VisAppBus::subscibeEvent(this, "PressDispSensorReady");
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setSpacing(15);

    this->setStyleSheet(R"(
                        QLabel#TitleLabel {
                        background-color: #BBD6EE;
                        border: 1px solid #A0C0E0;
                        padding: 5px;
                        font-size: 14px;
                        }
                        QGroupBox {
                        border: 1px solid #D0D0D0;
                        margin-top: 10px;
                        font-size: 13px;
                        color: #333333;
                        }
                        QGroupBox::title {
                        subcontrol-origin: margin;
                        left: 10px;
                        padding: 0 3px;
                        }
                        )");

    mainLayout->addWidget(CreatePressSensorPanel());
    mainLayout->addWidget(CreateDispSensorPanel());

    //日志区域
    QGroupBox *logGroup = new QGroupBox(u8"实时日志");
    QVBoxLayout *logLayout = new QVBoxLayout(logGroup);
    m_textLog = new QTextEdit();
    m_textLog->setReadOnly(true);
    logLayout->addWidget(m_textLog);
    mainLayout->addWidget(logGroup, 1);

    mainLayout->addStretch();
    this->setLayout(mainLayout);
    LoadUIParam();
}

WidgetPressDispDebug::~WidgetPressDispDebug()
{
}

void WidgetPressDispDebug::AppendLog(const QString& text)
{
    m_textLog->append(QString("[%1] %2").arg(QTime::currentTime().toString("hh:mm:ss")).arg(text));
}

QGroupBox* WidgetPressDispDebug::CreatePressSensorPanel()
{
    QGroupBox *group = new QGroupBox(u8"压力传感器（Modbus-RTU）", this);
    QVBoxLayout *vLayout = new QVBoxLayout(group);
    vLayout->setSpacing(8);

    QComboBox *cbxPort = new QComboBox(this);
    cbxPort->setObjectName("combox_StrpressPort");
    // 波特率/数据位/校验/停止位/从站地址从 hardwareParam 取，界面隐藏
    QComboBox *cbxBaud = new QComboBox(this);
    cbxBaud->setObjectName("combox_StrpressBaud");
    cbxBaud->hide();
    QSpinBox *spDataBit = new QSpinBox(this);
    spDataBit->setObjectName("spinBox_pressDataBits");
    spDataBit->hide();
    QComboBox *cbxParity = new QComboBox(this);
    cbxParity->setObjectName("combox_pressParity");
    cbxParity->hide();
    QSpinBox *spStopBit = new QSpinBox(this);
    spStopBit->setObjectName("spinBox_pressStopBits");
    spStopBit->hide();
    QSpinBox *spSlave = new QSpinBox(this);
    spSlave->setObjectName("spinBox_pressSlaveID");
    spSlave->hide();

    // 按钮行：端口 + 按钮
    QHBoxLayout *btnLayout = new QHBoxLayout();
    QPushButton *btnRefreshPort = new QPushButton(u8"刷新串口");
    QPushButton *btnConnect = new QPushButton(u8"连接");
    btnConnect->setCheckable(true);
    btnConnect->setEnabled(false);  //初始化完成前禁用,防止重复连接
    btnConnect->setObjectName("btn_PressConnect");
    QPushButton *btnReadReal = new QPushButton(u8"读取实时压力");
    QPushButton *btnReadPeak = new QPushButton(u8"读取峰值压力");
    QPushButton *btnResetPeak = new QPushButton(u8"清零峰值");

    btnLayout->addWidget(new QLabel(u8"端口:"));
    btnLayout->addWidget(cbxPort);
    btnLayout->addWidget(btnRefreshPort);
    btnLayout->addWidget(btnConnect);
    btnLayout->addWidget(btnReadReal);
    btnLayout->addWidget(btnReadPeak);
    btnLayout->addWidget(btnResetPeak);

    vLayout->addLayout(btnLayout);

    //刷新串口
    auto refreshFunc = [=](){
        cbxPort->clear();
        for(const auto& info : QSerialPortInfo::availablePorts())
            cbxPort->addItem(info.portName());
    };
    refreshFunc();
    connect(btnRefreshPort, &QPushButton::clicked, this, refreshFunc);

    //连接/断开
    connect(btnConnect, &QPushButton::clicked, this, [=](){
        bool checked = btnConnect->isChecked();
        QString port = cbxPort->currentText();
        auto& param = GlobalParam->hardwareParam.pressDispDebugParam;
        int baud = param.StrpressBaud.toInt();
        int databit = param.pressDataBits;
        int parity = param.pressParity;
        int stopbit = param.pressStopBits;
        int slave = param.pressSlaveID;

        if(checked)
        {
            AppendLog(QString(u8"压力传感器发起连接：%1").arg(port));
            QVariantMap setting;
            setting["PortName"] = port;
            setting["BaudRate"] = baud;
            setting["DataBits"] = databit;
            setting["Parity"] = parity;
            setting["StopBits"] = stopbit;
            setting["SlaveID"] = slave;
            QVariant paramVar(setting);
            int ret = VisAppBus::sendEvent("PressureSensorConnect", QString("TearStation"), paramVar);
            if(ret == 0)
            {
                btnConnect->setText(u8"断开");
                AppendLog(u8"压力传感器连接指令下发成功");
            }
            else
            {
                btnConnect->setChecked(false);
                AppendLog(u8"压力传感器连接指令下发失败");
            }
        }
        else
        {
            AppendLog(u8"压力传感器发起断开");
            VisAppBus::sendEvent("PressureSensorDisconnect", QString("TearStation"));
            btnConnect->setText(u8"连接");
        }
    });
    //读取实时压力
    connect(btnReadReal, &QPushButton::clicked, this, [=](){
        AppendLog(u8"下发指令：读取实时压力");
        float pressureVal = 0.0f;
        int ret = VisAppBus::sendEvent("PressureSensorGetRealPressure", QString("TearStation"), pressureVal);
        AppendLog(ret == 0 ? u8"指令发送成功" : u8"指令发送失败");
    });
    //读取峰值压力
    connect(btnReadPeak, &QPushButton::clicked, this, [=](){
        AppendLog(u8"下发指令：读取峰值压力");
        float peakVal = 0.0f;
        int ret = VisAppBus::sendEvent("PressureSensorGetPeakPressure", QString("TearStation"), peakVal);
        AppendLog(ret == 0 ? u8"指令发送成功" : u8"指令发送失败");
    });
    //清零峰值
    connect(btnResetPeak, &QPushButton::clicked, this, [=](){
        AppendLog(u8"下发指令：清零峰值谷值");
        int ret = VisAppBus::sendEvent("PressureSensorResetPeak", QString("TearStation"));
        AppendLog(ret == 0 ? u8"指令发送成功" : u8"指令发送失败");
    });

    return group;
}

QGroupBox* WidgetPressDispDebug::CreateDispSensorPanel()
{
    QGroupBox *group = new QGroupBox(u8"位移传感器（DL-RS1A ASCII）", this);
    QVBoxLayout *vLayout = new QVBoxLayout(group);
    vLayout->setSpacing(8);

    QComboBox *cbxPort = new QComboBox(this);
    cbxPort->setObjectName("combox_StrdispPort");
    // 波特率/数据位/校验/停止位从 hardwareParam 取，界面隐藏
    QComboBox *cbxBaud = new QComboBox(this);
    cbxBaud->setObjectName("combox_StrdispBaud");
    cbxBaud->hide();
    QSpinBox *spDataBit = new QSpinBox(this);
    spDataBit->setObjectName("spinBox_dispDataBits");
    spDataBit->hide();
    QComboBox *cbxParity = new QComboBox(this);
    cbxParity->setObjectName("combox_dispParity");
    cbxParity->hide();
    QSpinBox *spStopBit = new QSpinBox(this);
    spStopBit->setObjectName("spinBox_dispStopBits");
    spStopBit->hide();

    // 按钮行：端口 + 按钮
    QHBoxLayout *btnLayout = new QHBoxLayout();
    QPushButton *btnRefreshPort = new QPushButton(u8"刷新串口");
    QPushButton *btnConnect = new QPushButton(u8"连接");
    btnConnect->setCheckable(true);
    btnConnect->setEnabled(false);  //初始化完成前禁用,防止重复连接
    btnConnect->setObjectName("btn_DispConnect");
    QPushButton *btnReadHeight = new QPushButton(u8"读取高度");
    QPushButton *btnResetZero = new QPushButton(u8"传感器置零");

    btnLayout->addWidget(new QLabel(u8"端口:"));
    btnLayout->addWidget(cbxPort);
    btnLayout->addWidget(btnRefreshPort);
    btnLayout->addWidget(btnConnect);
    btnLayout->addWidget(btnReadHeight);
    btnLayout->addWidget(btnResetZero);

    vLayout->addLayout(btnLayout);

    //刷新串口
    auto refreshFunc = [=](){
        cbxPort->clear();
        for(const auto& info : QSerialPortInfo::availablePorts())
            cbxPort->addItem(info.portName());
    };
    refreshFunc();
    connect(btnRefreshPort, &QPushButton::clicked, this, refreshFunc);

    //连接断开
    connect(btnConnect, &QPushButton::clicked, this, [=](){
        bool checked = btnConnect->isChecked();
        QString port = cbxPort->currentText();
        auto& param = GlobalParam->hardwareParam.pressDispDebugParam;
        int baud = param.StrdispBaud.toInt();
        int databit = param.dispDataBits;
        int parity = param.dispParity;
        int stopbit = param.dispStopBits;

        if(checked)
        {
            AppendLog(QString(u8"位移传感器发起连接：%1").arg(port));
            QVariantMap setting;
            setting["PortName"] = port;
            setting["BaudRate"] = baud;
            setting["DataBits"] = databit;
            setting["Parity"] = parity;
            setting["StopBits"] = stopbit;
            QVariant paramVar(setting);
            int ret = VisAppBus::sendEvent("DisplacementSensorConnect", QString("TearStation"), paramVar);
            if(ret == 0)
            {
                btnConnect->setText(u8"断开");
                AppendLog(u8"位移传感器连接指令下发成功");
            }
            else
            {
                btnConnect->setChecked(false);
                AppendLog(u8"位移传感器连接指令下发失败");
            }
        }
        else
        {
            AppendLog(u8"位移传感器发起断开");
            VisAppBus::sendEvent("DisplacementSensorDisconnect", QString("TearStation"));
            btnConnect->setText(u8"连接");
        }
    });

    //读取高度
    connect(btnReadHeight, &QPushButton::clicked, this, [=](){
        AppendLog(u8"下发指令：读取位移高度");
        QString sensorId = "L1";
        double h = 0.0;
        int ret = VisAppBus::sendEvent("DisplacementSensorReadHeight", QString("TearStation"), sensorId, h);
        if(ret == 0)
        {
            AppendLog(QString(u8"指令发送成功，当前高度：%1").arg(h,0,'f',3));
        }
        else
        {
            AppendLog(u8"指令发送失败");
        }
    });

    //置零
    connect(btnResetZero, &QPushButton::clicked, this, [=](){
        AppendLog(u8"下发指令：传感器置零");
        int ret = VisAppBus::sendEvent("DisplacementSensorResetZero", QString("TearStation"));
        AppendLog(ret == 0 ? u8"指令发送成功" : u8"指令发送失败");
    });

    return group;
}

void WidgetPressDispDebug::LoadUIParam()
{
    QString filename = GlobalParam->hardwareParam.fileName;
    QSettings *config = new QSettings(filename, QSettings::IniFormat);
    config->setIniCodec(QTextCodec::codecForName("UTF-8"));
    config->beginGroup(QString("PressDispDebugParam"));
    VisUIParam::LoadIniToUI(config, this, &GlobalParam->hardwareParam.pressDispDebugParam);
    config->endGroup();
    delete config;
}

void WidgetPressDispDebug::UpdateParamToUI()
{
    LoadUIParam();
}

void WidgetPressDispDebug::SaveUIParam()
{
    auto& p = GlobalParam->hardwareParam.pressDispDebugParam;
    // COM口从界面控件读回（界面只能改COM口）
    auto* cbxPressPort = findChild<QComboBox*>("combox_StrpressPort");
    if (cbxPressPort) p.StrpressPort = cbxPressPort->currentText();
    auto* cbxDispPort = findChild<QComboBox*>("combox_StrdispPort");
    if (cbxDispPort) p.StrdispPort = cbxDispPort->currentText();

    QString filename = GlobalParam->hardwareParam.fileName;
    QSettings *config = new QSettings(filename, QSettings::IniFormat);
    config->setIniCodec(QTextCodec::codecForName("UTF-8"));
    config->beginGroup(QString("PressDispDebugParam"));
    // 直接从对象属性写配置（不依赖UI控件，避免隐藏控件无值被清空）
    config->setValue("StrpressPort", p.StrpressPort);
    config->setValue("StrpressBaud", p.StrpressBaud);
    config->setValue("pressDataBits", p.pressDataBits);
    config->setValue("pressParity", p.pressParity);
    config->setValue("pressStopBits", p.pressStopBits);
    config->setValue("pressSlaveID", p.pressSlaveID);
    config->setValue("StrdispPort", p.StrdispPort);
    config->setValue("StrdispBaud", p.StrdispBaud);
    config->setValue("dispDataBits", p.dispDataBits);
    config->setValue("dispParity", p.dispParity);
    config->setValue("dispStopBits", p.dispStopBits);
    config->endGroup();
    config->sync();
    delete config;
}

int WidgetPressDispDebug::event_PressDispSensorReady()
{
    //压力连接按钮
    auto* btnPress = findChild<QPushButton*>("btn_PressConnect");
    if (btnPress) {
        btnPress->setEnabled(true);
        btnPress->setChecked(true);
        btnPress->setText(u8"断开");
    }
    //位移连接按钮
    auto* btnDisp = findChild<QPushButton*>("btn_DispConnect");
    if (btnDisp) {
        btnDisp->setEnabled(true);
        btnDisp->setChecked(true);
        btnDisp->setText(u8"断开");
    }
    return 0;
}
