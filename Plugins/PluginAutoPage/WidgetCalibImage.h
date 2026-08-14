#ifndef WidgetCalibImage_H
#define WidgetCalibImage_H

#include <QWidget>
#include "opencv2/opencv.hpp"
#include "../../interface/coreinterface.h"

namespace Ui {
class WidgetCalibImage;
}

class WidgetCalibImage : public QWidget
{
    Q_OBJECT

public:
    explicit WidgetCalibImage(QWidget *parent = nullptr);
    ~WidgetCalibImage();

    void Init();

public slots:
    int event_ShowCalibImage(int station, cv::Mat mat);
    int event_ShowResultImage(int station, QString ngInfo);

protected:
    void CreateOKImg(int station, bool ok);

private:
    Ui::WidgetCalibImage *ui;
};

#endif // WidgetCalibImage_H
