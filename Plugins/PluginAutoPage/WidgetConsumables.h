#ifndef WidgetConsumables_H
#define WidgetConsumables_H

#include <QWidget>
#include <QTimer>
#include "ParamDef.h"
#include "WearingPartsList.h"
#include "WearingPartsListUser.h"

namespace Ui {
class WidgetConsumables;
}

class WidgetConsumables : public QWidget
{
    Q_OBJECT

public:
    explicit WidgetConsumables(QWidget *parent = nullptr);
    ~WidgetConsumables();

public slots:
    int event_ShowConsumables();
    int event_CalibComplete(QVector<ModuleInfo> &info);

private:
    void Init();

private:
    Ui::WidgetConsumables *ui;
    //易损件
     WearingPartsList m_wearingPartsList;
     WearingPartsListUser *m_wearingPartsListUser = nullptr;
};

#endif // WidgetConsumables_H
