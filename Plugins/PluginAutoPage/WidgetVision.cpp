#include "WidgetVision.h"
#include "ui_WidgetVision.h"
#include "VisAppBus.h"
#include "ParamManager.h"

WidgetVision::WidgetVision(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WidgetVision)
{
    ui->setupUi(this);
    qRegisterMetaType<QImage>("QImage");
    VisAppBus::subscibeEvent(this, "ShowDirtyImg");
}

WidgetVision::~WidgetVision()
{
    delete ui;
}

int WidgetVision::event_ShowDirtyImg(int station, QImage img)
{
    if (img.isNull()) return 0;
    ui->widgetDirtyImg->ClearText();
    ui->widgetDirtyImg->SetImage(img, false);
    ui->widgetDirtyImg->update();
    return 0;
}

void WidgetVision::on_btnTestDirty_clicked()
{
    QImage img(100, 100, QImage::Format_RGB888);
    img.fill(Qt::black);
    ui->widgetDirtyImg->ClearText();
    ui->widgetDirtyImg->SetImage(img, false);
    UserImageCanvas::TextParam textItem;
    textItem.text = QString("Test Dirty OK");
    textItem.color = Qt::green;
    textItem.drawAtTop = true;
    textItem.posIsImg = false;
    textItem.rect = QRectF(0, 0, 1000, 200);
    textItem.font.setPointSizeF(20);
    ui->widgetDirtyImg->AddText(textItem);
    ui->widgetDirtyImg->update();
}
