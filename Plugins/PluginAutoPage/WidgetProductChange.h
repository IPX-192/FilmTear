#ifndef WidgetProductChange_H
#define WidgetProductChange_H

#include <QWidget>
#include "ParamDef.h"
#include "opencv2/opencv.hpp"
#include "../../interface/coreinterface.h"

namespace Ui {
class WidgetProductChange;
}

class WidgetProductChange : public QWidget
{
    Q_OBJECT

public:
    explicit WidgetProductChange(QWidget *parent = nullptr);
    ~WidgetProductChange();

public slots:
    void  slot_InputInfo(tagOutputInfo& inputInfo);

private slots:
    void on_pushButton_clicked();

private:
    void SaveChangeInfo();
    void ReadChangeInfo();

private:
    Ui::WidgetProductChange *ui;
    int m_csvCount = 0;
};

#endif // WidgetProductChange_H
