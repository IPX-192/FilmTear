#include "WidgetManualCtl.h"
#include <QLabel>
#include <QSpacerItem>
#include <QSizePolicy>
#include <QTextCodec>
#include <QSettings>
#include <QMessageBox>
#include "VisAppBus.h"
#include "VisMotorManager.h"

using namespace VisMotorToolSpace;


WidgetManualCtl::WidgetManualCtl(QWidget *parent) : QWidget(parent)
{
    this->setWindowTitle("手动调试");
    this->setMaximumWidth(1200);

    this->setStyleSheet(R"(
                        QWidget {
                        background-color: #EAF7FF;
                        }
                        QGroupBox {
                        border: 1px solid #C8C8C8;
                        margin-top: 10px;
                        font-size: 13px;
                        color: #222222;
                        }
                        QGroupBox::title {
                        subcontrol-origin: margin;
                        left: 8px;
                        padding: 0 4px;
                        }
                        QComboBox {
                        background-color: #D0D0D0;
                        border: 1px solid #AAAAAA;
                        padding: 6px 10px;
                        min-width: 120px;
                        font-size:13px;
                        }
                        QPushButton {
                        background-color: #D0D0D0;
                        border: 1px solid #AAAAAA;
                        padding: 7px 22px;
                        font-size:13px;
                        }
                        QPushButton:pressed {
                        background-color: #32CD32;
                        color: white;
                        border: 1px solid #228B22;
                        }
                        QRadioButton {
                        font-size:13px;
                        }
                        )");

    QWidget* centralWidget = new QWidget(this);
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setSpacing(18);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    QVBoxLayout* outerLayout = new QVBoxLayout(this);
    outerLayout->addWidget(centralWidget, 0, Qt::AlignLeft);
    outerLayout->addStretch();
    outerLayout->setContentsMargins(0,0,0,0);

    // 壳体料仓 FeedHolder
    mainLayout->addWidget(CreateMaterialGroup("料仓", "combo_material", TrayFunc::FeedHolder));
    // PCB料仓 FeedPCB
    mainLayout->addWidget(CreateMaterialGroup("PCB", "combo_pcb", TrayFunc::FeedPCB));

    QWidget* bottomWidget = new QWidget();
    QVBoxLayout* bottomLayout = new QVBoxLayout(bottomWidget);
    bottomLayout->setContentsMargins(0, 10, 0, 0);
    bottomLayout->setSpacing(12);

    QWidget* jigLineWidget = new QWidget();
    QHBoxLayout* jigLineLayout = new QHBoxLayout(jigLineWidget);
    jigLineLayout->setContentsMargins(0,0,0,0);
    jigLineLayout->setSpacing(30);

    QRadioButton* radio1 = new QRadioButton("治具1");
    QRadioButton* radio2 = new QRadioButton("治具2");
    QRadioButton* radio3 = new QRadioButton("治具3");
    QRadioButton* radio4 = new QRadioButton("治具4");
    radio3->setChecked(true);

    QButtonGroup* jigGroup = new QButtonGroup(bottomWidget);
    jigGroup->addButton(radio1, 1);
    jigGroup->addButton(radio2, 2);
    jigGroup->addButton(radio3, 3);
    jigGroup->addButton(radio4, 4);

    QPushButton* btnWork = new QPushButton("作业");
    connect(btnWork, &QPushButton::clicked, this, [=](){
        int jigNo = jigGroup->checkedId();
        VisAppBus::sendEvent("ManualSelectFixture", jigNo);
    });

    jigLineLayout->addWidget(radio1);
    jigLineLayout->addWidget(radio2);
    jigLineLayout->addWidget(radio3);
    jigLineLayout->addWidget(radio4);
    jigLineLayout->addStretch(3);
    jigLineLayout->addWidget(btnWork, 1);

    QWidget* cleanLineWidget = new QWidget();
    QHBoxLayout* cleanLineLayout = new QHBoxLayout(cleanLineWidget);
    cleanLineLayout->setContentsMargins(110,0,0,0);

    QPushButton* btnClean = new QPushButton("清洗");
    connect(btnClean, &QPushButton::clicked, this, [=](){
		/* TrayInfo emptyTray;
		 emptyTray.empty = false;
		 emptyTray.pcbBarCode = "PCB111";
		 VisAppBus::sendEvent("PipeLineTrayReady", TrayFunc::PCBClean, emptyTray);*/
		static int index = 1;
		TrayInfo emptyTray;
		emptyTray.empty = false;
		emptyTray.pcbBarCode = QString("PCB%1").arg(index);
		emptyTray.holderBarCode= QString("Holder%1").arg(index);
		index++;
		VisAppBus::sendEvent("PipeLineTrayReady", TrayFunc::FeedTurntable, emptyTray);
		
    });

    cleanLineLayout->addStretch(3);

    cleanLineLayout->addWidget(btnClean, 1);

    //测试按钮：撕膜旋转R1——IN49绿灯(ON)开始顺时针旋转，变红灯(OFF)停止
    QPushButton* btnTest = new QPushButton("测试");
    connect(btnTest, &QPushButton::clicked, this, [=](){

    });
    cleanLineLayout->addWidget(btnTest, 1);

    bottomLayout->addWidget(jigLineWidget);
    bottomLayout->addWidget(cleanLineWidget);

    mainLayout->addWidget(bottomWidget);
    mainLayout->addStretch();
}

QGroupBox* WidgetManualCtl::CreateMaterialGroup(const QString& groupName, const QString& comboObjName, TrayFunc funcType)
{
    QGroupBox* groupBox = new QGroupBox(groupName);
    QHBoxLayout* layout = new QHBoxLayout(groupBox);
    layout->setSpacing(15);
    layout->setContentsMargins(15, 12, 15, 12);

    QComboBox* combo = new QComboBox();
    combo->setObjectName(comboObjName);
    combo->addItems({"第一层", "第二层", "第三层", "第四层","第五层"});
    combo->setCurrentIndex(0);

    QPushButton* btnPull = new QPushButton("拉料");
    QPushButton* btnBack = new QPushButton("退料");

    connect(btnPull, &QPushButton::clicked, this, [=](){
        int layerIdx = combo->currentIndex();
        VisAppBus::sendEvent("InTrayTaskLayer", funcType, layerIdx);
    });
    connect(btnBack, &QPushButton::clicked, this, [=](){
        int layerIdx = combo->currentIndex();
        VisAppBus::sendEvent("BlankTrayLayer", funcType, layerIdx);
    });

    layout->addWidget(combo);
    layout->addStretch(3);
    layout->addWidget(btnPull, 1);
    layout->addWidget(btnBack, 1);

    return groupBox;
}
