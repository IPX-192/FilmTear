#ifndef WidgetVision_H
#define WidgetVision_H

#include <QWidget>
#include "opencv2/opencv.hpp"
#include "ImageCanvas.h"

namespace Ui {
class WidgetVision;
}

class WidgetVision : public QWidget
{
    Q_OBJECT

public:
    explicit WidgetVision(QWidget *parent = nullptr);
    ~WidgetVision();

public slots:
    int event_ShowDirtyImg(int station, QImage img);

private slots:
    void on_btnTestDirty_clicked();

private:
    Ui::WidgetVision *ui;
};

#endif // WidgetVision_H
