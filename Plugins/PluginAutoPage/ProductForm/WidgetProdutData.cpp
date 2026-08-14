#include "WidgetProdutData.h"
#include "ui_WidgetProdutData.h"
#include "ProductDetailForm.h"
#include "VisAppBus.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QGroupBox>

WidgetProdutData::WidgetProdutData(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WidgetProdutData)
{
    ui->setupUi(this);
    qRegisterMetaType<cv::Mat>("cv::Mat");
    qRegisterMetaType<QImage>("QImage");
    InitTabs();

    VisAppBus::subscibeEvent(this, "ClearProductData");
    VisAppBus::subscibeEvent(this, "AddProductData");
    VisAppBus::subscibeEvent(this, "ShowCalibImage");
    VisAppBus::subscibeEvent(this, "ShowDirtyImg");
    VisAppBus::subscibeEvent(this, "ShowResultImage");
}

WidgetProdutData::~WidgetProdutData()
{
    delete ui;
}

void WidgetProdutData::InitTabs()
{
    m_tabWidget = new QTabWidget(this);
    ui->mainLayout->addWidget(m_tabWidget);

    for (int i = 0; i < 4; i++) {
        // 为每个治具创建一个 tab 页面
        QWidget* tabPage = new QWidget();
        QHBoxLayout* hLayout = new QHBoxLayout(tabPage);
        hLayout->setContentsMargins(5, 5, 5, 5);
        hLayout->setSpacing(8);

        // 左侧：数据表格（占 60% 宽度）
        m_vecProductDetail[i] = new ProductDetailForm(i, tabPage);
        hLayout->addWidget(m_vecProductDetail[i], 3);

        // 右侧：对位图像 + 脏污图像（垂直排列）
        QWidget* rightPanel = new QWidget(tabPage);
        QVBoxLayout* vLayout = new QVBoxLayout(rightPanel);
        vLayout->setContentsMargins(0, 0, 0, 0);
        vLayout->setSpacing(6);

        // 对位图像
        QGroupBox* groupAlign = new QGroupBox(u8"对位图像", rightPanel);
        QVBoxLayout* alignLayout = new QVBoxLayout(groupAlign);
        m_vecAlignImg[i] = new ImageCanvas(groupAlign);
        m_vecAlignImg[i]->setMinimumSize(280, 200);
        alignLayout->addWidget(m_vecAlignImg[i]);
        vLayout->addWidget(groupAlign);

        // 脏污图像 + 测试按钮
        QGroupBox* groupDirty = new QGroupBox(u8"脏污图像", rightPanel);
        QVBoxLayout* dirtyLayout = new QVBoxLayout(groupDirty);
        m_vecDirtyImg[i] = new ImageCanvas(groupDirty);
        m_vecDirtyImg[i]->setMinimumSize(280, 200);
        dirtyLayout->addWidget(m_vecDirtyImg[i]);

        QPushButton* btnTest = new QPushButton(u8"测试", groupDirty);
        int station = i;
        QObject::connect(btnTest, &QPushButton::clicked, this, [=](){
            QImage img(100, 100, QImage::Format_RGB888);
            img.fill(Qt::black);
            m_vecDirtyImg[station]->ClearText();
            m_vecDirtyImg[station]->SetImage(img, false);
            UserImageCanvas::TextParam textItem;
            textItem.text = QString("Test Dirty OK");
            textItem.color = Qt::green;
            textItem.drawAtTop = true;
            textItem.posIsImg = false;
            textItem.rect = QRectF(0, 0, 1000, 200);
            textItem.font.setPointSizeF(20);
            m_vecDirtyImg[station]->AddText(textItem);
            m_vecDirtyImg[station]->update();
        });
        dirtyLayout->addWidget(btnTest);
        vLayout->addWidget(groupDirty);

        hLayout->addWidget(rightPanel, 2);

        m_tabWidget->addTab(tabPage, QString(u8"治具%1").arg(i + 1));
    }
}

int WidgetProdutData::event_InitProductData(int station, QString barCode)
{
    if (station >= 0 && station < 4)
        m_vecProductDetail[station]->InitData(barCode);
    return 0;
}

int WidgetProdutData::event_AddProductData(int station)
{
    if (station < 0 || station >= 4) return 0;
    ItemDetail info;
    info.result = true;  // 空跑/默认
    m_vecProductDetail[station]->AddData(info);
    return 0;
}

int WidgetProdutData::event_ShowCalibImage(int station, cv::Mat mat)
{
    if (station < 0 || station >= 4) return 0;
    QImage image = QImage(static_cast<const unsigned char *>(mat.data),
                          mat.cols, mat.rows, static_cast<int>(mat.step),
                          QImage::Format_RGB888).copy();
    m_vecAlignImg[station]->ClearText();
    m_vecAlignImg[station]->SetImage(image, false);
    m_vecAlignImg[station]->update();
    return 0;
}

int WidgetProdutData::event_ShowDirtyImg(int station, QImage img)
{
    if (station < 0 || station >= 4) return 0;
    if (img.isNull()) return 0;
    m_vecDirtyImg[station]->ClearText();
    m_vecDirtyImg[station]->SetImage(img, false);
    m_vecDirtyImg[station]->update();
    return 0;
}

int WidgetProdutData::event_ShowResultImage(int station, QString ngInfo)
{
    if (station < 0 || station >= 4) return 0;
    ImageCanvas* widgetImg = m_vecDirtyImg[station];
    widgetImg->ClearText();
    UserImageCanvas::TextParam textItem;
    textItem.text = ngInfo.contains("OK") ? u8"OK" : QString(u8"NG:%1").arg(ngInfo);
    textItem.color = ngInfo.contains("OK") ? Qt::green : Qt::red;
    textItem.drawAtTop = true;
    textItem.posIsImg = false;
    textItem.rect = QRectF(0, 0, 1000, 200);
    textItem.font.setPointSizeF(20);
    widgetImg->AddText(textItem);
    widgetImg->update();
    return 0;
}
