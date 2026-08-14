#ifndef WIDGETMANUALDEBUG_H
#define WIDGETMANUALDEBUG_H

#include <QWidget>
#include "../../interface/coreinterface.h"

namespace Ui {
class WidgetManualDebug;
}

class QToolButton;
class WidgetManualDebug :public QWidget
{
    Q_OBJECT

public:
    explicit WidgetManualDebug(QWidget *parent = nullptr);
    ~WidgetManualDebug();
    void  AddLog(QString msg, LogLevel level);
	void  LoadUIParam();

protected slots:
	int  event_LoginUserChange();

private slots:
	void on_btnUpdateUI_clicked();

	void on_btnSaveParam_clicked();

private:
    Ui::WidgetManualDebug *ui;
};

#endif // WIDGETMANUALDEBUG_H
