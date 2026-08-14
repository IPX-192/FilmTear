#ifndef WIDGETAUTOPAGE_H
#define WIDGETAUTOPAGE_H

#include <QWidget>
#include "../../interface/coreinterface.h"

namespace Ui {
class WidgetAutoPage;
}

class QAbstractButton;
class WidgetLogAll;
class WidgetAutoPage :public QWidget
{
    Q_OBJECT

public:
    explicit WidgetAutoPage(QWidget *parent = nullptr);
    ~WidgetAutoPage();

public:
    void  InitLog(QList<PluginLogInfo>& listPluginLog);
   
protected:
    WidgetLogAll* m_widgetLogAll;

private:
    Ui::WidgetAutoPage *ui;
};

#endif // WIDGETAUTOPAGE_H
