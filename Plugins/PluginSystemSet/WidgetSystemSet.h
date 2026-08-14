#ifndef WIDGETSYSTEMSET_H
#define WIDGETSYSTEMSET_H

#include <QWidget>

namespace Ui {
class WidgetSystemSet;
}

class WidgetSystemSet :public QWidget
{
    Q_OBJECT

public:
    explicit WidgetSystemSet(QWidget *parent = nullptr);
    ~WidgetSystemSet();

public:
	void  LoadUIParam();
	void  UpdateUIParam();

protected slots:
	int  event_LoginUserChange();

private slots:
	void on_btnUpdateUI_clicked();

	void on_btnSaveParam_clicked();

private:
    Ui::WidgetSystemSet *ui;
};

#endif // WIDGETSYSTEMSET_H
