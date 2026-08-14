#ifndef WIDGETLOGALL_H
#define WIDGETLOGALL_H

#include <QWidget>
#include "../../interface/coreinterface.h"

namespace Ui {
class WidgetLogAll;
}

class WidgetLogAll : public QWidget
{
    Q_OBJECT

public:
    explicit WidgetLogAll(QWidget *parent = nullptr);
    ~WidgetLogAll();

    void  InitLog(QList<PluginLogInfo>& listPluginLog);

private:
    Ui::WidgetLogAll *ui;
};

#endif // WIDGETLOGALL_H
