#ifndef WIDGETTRAYSCANDEBUG_H
#define WIDGETTRAYSCANDEBUG_H

#include <QWidget>
#include "TrayRfidForm.h"
namespace Ui {
class WidgetTrayScanDebug;
}

class WidgetTrayScanDebug : public QWidget
{
    Q_OBJECT
public:
    explicit WidgetTrayScanDebug(QWidget *parent = nullptr);
    ~WidgetTrayScanDebug() override;

public slots:
    void LoadUIParam();
    void UpdateParamToUI();
    void SaveUIParam();

private:
    Ui::WidgetTrayScanDebug *ui;
    TrayRfidForm* m_trayRfidForm;
};

#endif // WIDGETTRAYSCANDEBUG_H
