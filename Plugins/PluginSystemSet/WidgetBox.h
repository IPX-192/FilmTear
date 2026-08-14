#ifndef WIDGETBOX_H
#define WIDGETBOX_H

#include <QWidget>

namespace Ui {
class WidgetBox;
}

class WidgetBox : public QWidget
{
    Q_OBJECT

public:
    explicit WidgetBox(QWidget *parent = nullptr);
    ~WidgetBox();

public slots:
	void  LoadUIParam();
	void  SaveUIParam();
	void  UpdateParamToUI();

private:
    Ui::WidgetBox *ui;
};

#endif // WIDGETBOX_H
