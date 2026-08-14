#ifndef WIDGETGRIP_H
#define WIDGETGRIP_H

#include <QWidget>

namespace Ui {
class WidgetGrip;
}

class WidgetGrip : public QWidget
{
    Q_OBJECT

public:
    explicit WidgetGrip(QWidget *parent = nullptr);
    ~WidgetGrip();

public slots:
	void  LoadUIParam();
	void  SaveUIParam();
	void  UpdateParamToUI();

private:
    Ui::WidgetGrip *ui;
};

#endif // WIDGETGRIP_H
