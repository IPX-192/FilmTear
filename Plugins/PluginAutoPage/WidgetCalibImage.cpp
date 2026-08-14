
#include "WidgetCalibImage.h"
#include "ui_WidgetCalibImage.h"
#include "ParamManager.h"
#include "../../Common/WidgetLog.h"

WidgetCalibImage::WidgetCalibImage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WidgetCalibImage)
{
    ui->setupUi(this);
    qRegisterMetaType<cv::Mat>("cv::Mat");
    VisAppBus::subscibeEvent(this, "ShowCalibImage");
    VisAppBus::subscibeEvent(this, "ShowResultImage");
}

WidgetCalibImage::~WidgetCalibImage()
{
    delete ui;
}

void WidgetCalibImage::Init()
{
    QVector<WidgetLog*>listLogWidget;
	listLogWidget << ui->widget_L << ui->widget_R;
	for (int i = 0; i < 2; i++)
	{
		PluginLogInfo pluginLog;
        pluginLog.type = Log_Fixture;
		pluginLog.index = i;
		pluginLog._pLog = std::bind(&WidgetLog::addLog, listLogWidget[i], std::placeholders::_1, std::placeholders::_2);
		GlobalParam->frameCore->listPluginLog.append(pluginLog);
	}
}

int WidgetCalibImage::event_ShowCalibImage(int station, cv::Mat mat)
{
    QImage image = QImage(static_cast<const unsigned char *>(mat.data), mat.cols, mat.rows, static_cast<int>(mat.step), QImage::Format_RGB888).copy();
    ImageCanvas *widgetImg=station?ui->widgetRealTimeImg2:ui->widgetRealTimeImg1;
    widgetImg->ClearText();
    widgetImg->SetImage(image, false);
    widgetImg->update();
    return 0;
}

int WidgetCalibImage::event_ShowResultImage(int station, QString ngInfo)
{
	QImage img(100, 100, QImage::Format_RGB888);
	img.fill(Qt::black);
    ImageCanvas* widgetImg = station ? ui->widgetRealTimeImg2 : ui->widgetRealTimeImg1;
    widgetImg->ClearText();
	widgetImg->SetImage(img, false);
    UserImageCanvas::TextParam textItem;
    textItem.text = ngInfo.isEmpty() ? "OK" : QString("NG:%1").arg(ngInfo);
	textItem.color = ngInfo.isEmpty() ? Qt::green : Qt::red;
    textItem.drawAtTop = true;
	textItem.posIsImg = false;
	textItem.rect = QRectF(0, 0, 1000, 200);
    textItem.font.setPointSizeF(20);
	widgetImg->AddText(textItem);
	widgetImg->update();
    return 0;
}

void WidgetCalibImage::CreateOKImg(int station, bool ok)
{
    // 创建一张 1920x1080 的黑色图像
    cv::Mat image = cv::Mat::zeros(1080, 1920, CV_8UC3);

    // 定义字体类型
    int font = cv::FONT_HERSHEY_SIMPLEX;
    int fontSize = 10;
    int thickness = 20;
    int baseline = 0;
    if (!ok)
    {
        std::string ngText = "NG";
        cv::Scalar redColor(255, 0, 0);
        cv::Size ngTextSize = cv::getTextSize(ngText, font, fontSize, thickness, &baseline);
        cv::Point ngTextOrg((image.cols - ngTextSize.width) / 2, (image.rows + ngTextSize.height) / 2 - baseline / 2);
        cv::putText(image, ngText, ngTextOrg, font, fontSize, redColor, thickness);
    }
    else
    {
        // 在图像上添加 "OK" 字样，绿色字体
        std::string okText = "OK";
        cv::Scalar greenColor(0, 255, 0);

        // 计算文本位置，确保在图像中心下方
        cv::Size okTextSize = cv::getTextSize(okText, font, fontSize, thickness, &baseline);
        cv::Point okTextOrg((image.cols - okTextSize.width) / 2, (image.rows + okTextSize.height) / 2 - baseline / 2);
        cv::putText(image, okText, okTextOrg, font, fontSize, greenColor, thickness);
    }
    VisAppBus::sendEvent("ShowCalibImage", station, image);
}
