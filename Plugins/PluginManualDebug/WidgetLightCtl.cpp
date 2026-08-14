#include "WidgetLightCtl.h"
#include "WidgetLightCtl.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QGroupBox>
#include <QButtonGroup>
#include <QComboBox>
#include <QComboBox>
#include <QMessageBox>
#include <QTextCodec>
#include <QSpinBox>
#include "ParamManager.h"
#include "VisUIParam.h"
#include "VisAppBus.h"

#pragma execution_character_set("utf-8")

WidgetLightCtl::WidgetLightCtl(QWidget *parent) : QWidget(parent)
{
    // 主控 Widget 和 布局
    this->setMaximumWidth(800);
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setSpacing(15);

    // 全局样式表：
    this->setStyleSheet(R"(
                        QLabel#TitleLabel {
                        background-color: #BBD6EE;
                        border: 1px solid #A0C0E0;
                        padding: 5px;
                        font-size: 14px;
                        }
                        QPushButton {
                        background-color: #A9A9A9;
                        color: white;
                        border: 2px solid #D0D0D0;
                        padding: 8px 20px;
                        font-size: 14px;
                        font-weight: bold;
                        }
                        QPushButton:checked {
                        background-color: #32CD32; /* 绿色 */
                        border: 2px solid #228B22;
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

    // 串口配置区域
    mainLayout->addWidget(CreateSerialConfigGroup("StrlightPort", "StrlightBaud"));

    //mainLayout->addWidget(CreateBasicSwitch("照明灯", false));
    //mainLayout->addWidget(CreateBasicSwitch("安全门", true));

    mainLayout->addWidget(CreateLightSource("上料视觉#光源<0>", "channel","lightValue"));
    //mainLayout->addWidget(createLightSource("上CCD光源3#光源<0>","spinbox_channel","spinbox_lightValue"));
    //mainLayout->addWidget(createLightSource("下CCD光源4#光源<0>","spinbox_channel","spinbox_lightValue"));

    mainLayout->addStretch();
    this->setLayout(mainLayout);
}

// --- 构建串口配置区域 ---
QWidget* WidgetLightCtl::CreateSerialConfigGroup(QString portObject, QString serBaudRateObject)
{
    QGroupBox *group = new QGroupBox("光源串口配置");
    QHBoxLayout *layout = new QHBoxLayout(group);

    QWidget *leftWidget = new QWidget();
    QHBoxLayout *leftLayout = new QHBoxLayout(leftWidget);
    leftLayout->setContentsMargins(0, 0, 0, 0);

    QComboBox *portCombo = new QComboBox();
    portCombo->setObjectName("combox_" + portObject);
    QComboBox *baudCombo = new QComboBox();
    baudCombo->setObjectName("combox_" + serBaudRateObject);
    baudCombo->addItems({"9600", "19200", "38400", "115200"});
    baudCombo->setCurrentText("115200");

    leftLayout->addWidget(new QLabel("端口:"));
    leftLayout->addWidget(portCombo, 1);
    leftLayout->addWidget(new QLabel("波特率:"));
    leftLayout->addWidget(baudCombo, 1);
    leftLayout->addStretch();

    QPushButton *connectBtn = new QPushButton("打开串口");
    connectBtn->setCheckable(true);
    connectBtn->setStyleSheet("QPushButton:checked { background-color: #E26A6A; border-color: #C0392B;}");

    QPushButton *refreshBtn = new QPushButton("刷新串口");
    refreshBtn->setStyleSheet(R"(
                              QPushButton {
                              background-color: #4A90E2; /* 默认蓝色 */
                              color: white;
                              border: 2px solid #357ABD;
                              padding: 8px 20px;
                              font-size: 14px;
                              font-weight: bold;
                              }
                              QPushButton:hover {
                              background-color: #357ABD; /* 鼠标悬停时加深 */
                              }
                              QPushButton:disabled {
                              background-color: #E0E0E0; /* 禁用时的背景色（浅灰） */
                              color: #A0A0A0;            /* 禁用时的文字颜色（深灰） */
                              border: 2px solid #CCCCCC; /* 禁用时的边框 */
                              }
                              )");

    layout->addWidget(leftWidget, 3);
    layout->addWidget(connectBtn, 1);
    layout->addWidget(refreshBtn, 1);

    auto refreshPorts = [portCombo]() {
        portCombo->clear();
        for (const QSerialPortInfo &info : QSerialPortInfo::availablePorts()) {
            portCombo->addItem(info.portName());
        }
    };


    refreshPorts();
    connect(refreshBtn, &QPushButton::clicked, this, refreshPorts);

    connect(connectBtn, &QPushButton::clicked, this, [=](){
        bool checked = connectBtn->isChecked();
        if(checked) {
            if(OpenLightSerial(portCombo->currentText(), baudCombo->currentText().toInt())){
                connectBtn->setText("关闭串口");
                m_serialOpened = true;
                portCombo->setEnabled(false);
                baudCombo->setEnabled(false);
                refreshBtn->setEnabled(false);
            } 
            else {
                QMessageBox::warning(this, "错误", "无法打开串口！请检查是否被占用。");
                connectBtn->setChecked(false);
            }
        } 
        else {
            CloseLightSerial();
            m_serialOpened = false;
            connectBtn->setText("打开串口");
            // 串口关闭时，恢复配置和刷新按钮
            portCombo->setEnabled(true);
            baudCombo->setEnabled(true);
            refreshBtn->setEnabled(true);
        }
    });

    return group;
}

QWidget* WidgetLightCtl::CreateBasicSwitch(QString title, bool isOpened)
{
    QWidget *widget = new QWidget();
    QHBoxLayout *layout = new QHBoxLayout(widget);
    layout->setContentsMargins(0, 0, 6, 0);

    QLabel *titleLabel = new QLabel(title);
    titleLabel->setObjectName("TitleLabel"); // 应用QSS样式
    titleLabel->setAlignment(Qt::AlignCenter);

    QPushButton *btnOpen = new QPushButton("打开");
    QPushButton *btnClose = new QPushButton("关闭");
    btnOpen->setCheckable(true);
    btnClose->setCheckable(true);

    // 使用按钮组实现互斥
    QButtonGroup *btnGroup = new QButtonGroup(widget);
    btnGroup->addButton(btnOpen, 1);
    btnGroup->addButton(btnClose, 0);

    // 设置初始状态
    if(isOpened) btnOpen->setChecked(true);
    else btnClose->setChecked(true);

    layout->addWidget(titleLabel, 3);
    layout->addWidget(btnOpen, 1);
    layout->addWidget(btnClose, 1);

    return widget;
}

QWidget* WidgetLightCtl::CreateLightSource(QString title,QString channelObject,QString lightValueObject) {
    QGroupBox *group = new QGroupBox(title);
    QHBoxLayout *layout = new QHBoxLayout(group);


    QWidget *leftWidget = new QWidget();
    QHBoxLayout *leftLayout = new QHBoxLayout(leftWidget);
    leftLayout->setContentsMargins(0, 0, 0, 0);

    QLabel *chLabel = new QLabel("通道:");
    QSpinBox *chBox = new QSpinBox();
    chBox->setRange(0, 16);
    chBox->setValue(0);
    chBox->setFixedWidth(50);
    chBox->setObjectName("spinBox_" + channelObject);

    QWidget *sliderContainer = new QWidget();
    QHBoxLayout *sliderLayout = new QHBoxLayout(sliderContainer);
    sliderLayout->setContentsMargins(5, 2, 5, 2);

    QSlider *slider = new QSlider(Qt::Horizontal);
    slider->setRange(0, 255);
    slider->setValue(0);
    slider->setObjectName("slider_" + lightValueObject);
    sliderLayout->addWidget(slider);

    leftLayout->addWidget(chLabel);
    leftLayout->addWidget(chBox);
    leftLayout->addWidget(sliderContainer, 1);

    QPushButton *btnOpen = new QPushButton("打开");
    QPushButton *btnClose = new QPushButton("关闭");
    btnOpen->setCheckable(true);
    btnClose->setCheckable(true);
    btnClose->setChecked(true); // 默认关闭

    QButtonGroup *btnGroup = new QButtonGroup(group);
    btnGroup->addButton(btnOpen, 1);
    btnGroup->addButton(btnClose, 0);

    layout->addWidget(leftWidget, 3);
    layout->addWidget(btnOpen, 1);
    layout->addWidget(btnClose, 1);

    // 标题实时更新
    connect(slider, &QSlider::valueChanged, this, [=](int value){
        QString newTitle = title;
        newTitle.replace(QRegExp("<\\d+>"), QString("<%1>").arg(value));
        group->setTitle(newTitle);
    });

    // 松开滑块才下发指令
    connect(slider, &QSlider::sliderReleased, this, [=](){
        if(btnOpen->isChecked()) {
            SendLightCommand(chBox->value(), slider->value());
        }
    });

    connect(btnGroup, QOverload<int>::of(&QButtonGroup::buttonClicked), this, [=](int btnId){
        SendLightCommand(chBox->value(), (btnId == 1) ? slider->value() : 0);
    });


    connect(chBox, QOverload<int>::of(&QSpinBox::valueChanged), this, [=](int newChannel){
        if(btnOpen->isChecked()) {
            SendLightCommand(newChannel, slider->value());
        }
    });

    return group;
}

void WidgetLightCtl::LoadUIParam()
{
    QString filename = GlobalParam->hardwareParam.fileName;
    QSettings *config = new QSettings(filename, QSettings::IniFormat);
    config->setIniCodec(QTextCodec::codecForName("UTF-8"));
    config->beginGroup(QString("LightParam"));
    VisUIParam::LoadIniToUI(config, this, &GlobalParam->hardwareParam.lightParam);
    config->endGroup();
    delete config;
}

void WidgetLightCtl::UpdateParamToUI()
{
    LoadUIParam();
}

void WidgetLightCtl::SaveUIParam()
{
    QString filename = GlobalParam->hardwareParam.fileName;
    QSettings *config = new QSettings(filename, QSettings::IniFormat);
    config->setIniCodec(QTextCodec::codecForName("UTF-8"));
    config->beginGroup(QString("LightParam"));
    VisUIParam::SaveUIToIni(config, this, &GlobalParam->hardwareParam.lightParam);
    config->endGroup();
    delete config;
}

bool WidgetLightCtl::OpenLightSerial(const QString& portName, int baudRate)
{
    int ret = VisAppBus::sendEvent("InitLightPort", portName, baudRate);
    return (ret == 0);
}

void WidgetLightCtl::CloseLightSerial()
{
    VisAppBus::sendEvent("UnInitLightPortAll");
}

void WidgetLightCtl::SendLightCommand(int lightId, int brightness)
{
    if (!m_serialOpened) {
        QMessageBox::warning(this, u8"提示", u8"请先打开串口！");
        return;
    }
    auto& param = GlobalParam->hardwareParam.lightParam;
    QString portName = param.StrlightPort;
    if (brightness == 0)
        VisAppBus::sendEvent("TurnOffLight", portName, lightId);
    else
        VisAppBus::sendEvent("SetLightValue", portName, lightId, brightness);
}
