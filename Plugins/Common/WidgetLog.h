#ifndef WIDGETLOG_H
#define WIDGETLOG_H

#include <QWidget>
#include "../../interface/coreinterface.h"

namespace Ui {
class WidgetLog;
}

class WidgetLog : public QWidget
{
    Q_OBJECT

public:
    explicit WidgetLog(QWidget *parent = nullptr);
    ~WidgetLog();

public:
    void addLog(QString msg, LogLevel level);
    void clearLog();

protected:
    int  m_rowTotal = 0;
 
private slots:
    void on_toolButton_clear_clicked();

private:
    Ui::WidgetLog *ui;
};

#endif // WIDGETLOG_H
