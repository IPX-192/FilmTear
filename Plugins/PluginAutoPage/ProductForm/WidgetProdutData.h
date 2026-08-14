#ifndef WIDGETPRODUTDATA_H
#define WIDGETPRODUTDATA_H

#include <QWidget>
#include <QTabWidget>
#include "opencv2/opencv.hpp"
#include "ImageCanvas.h"
#include "ParamManager.h"

namespace Ui {
class WidgetProdutData;
}
class ProductDetailForm;

class WidgetProdutData : public QWidget
{
    Q_OBJECT

public:
    explicit WidgetProdutData(QWidget *parent = nullptr);
    ~WidgetProdutData();

protected:
    void InitTabs();

public slots:
    int  event_InitProductData(int station, QString barCode);
    int  event_AddProductData(int station);
    int  event_ShowCalibImage(int station, cv::Mat mat);
    int  event_ShowDirtyImg(int station, QImage img);
    int  event_ShowResultImage(int station, QString ngInfo);

private:
    Ui::WidgetProdutData *ui;
    QTabWidget*           m_tabWidget;
    ProductDetailForm*    m_vecProductDetail[4];
    ImageCanvas*          m_vecAlignImg[4];
    ImageCanvas*          m_vecDirtyImg[4];
};

#endif // WIDGETPRODUTDATA_H
