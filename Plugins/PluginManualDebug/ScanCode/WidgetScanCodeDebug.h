#ifndef WIDGETSCANCODEDEBUG_H
#define WIDGETSCANCODEDEBUG_H

#include <QWidget>
#include <QSharedPointer>
#include "ParamManager.h"

namespace Ui {
class WidgetScanCodeDebug;
}

class ScanCodeForm;
class WidgetScanCodeDebug : public QWidget
{
    Q_OBJECT

public:
    explicit WidgetScanCodeDebug(QWidget *parent = nullptr);
    ~WidgetScanCodeDebug();
    void InitWidget();

public slots:
    void LoadUIParam();
    void UpdateParamToUI();
    void SaveUIParam();
private:


private:
    Ui::WidgetScanCodeDebug *ui;
    QList<ScanCodeForm*> m_scanCodeFormList;
};

#endif // WIDGETSCANCODEDEBUG_H
