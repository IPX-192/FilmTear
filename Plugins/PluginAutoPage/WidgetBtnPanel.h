#ifndef WIDGETBTNPANEL_H
#define WIDGETBTNPANEL_H

#include <QWidget>
#include "ParamDef.h"

namespace Ui {
class WidgetBtnPanel;
}

class SwitchButton;
class WidgetBtnPanel : public QWidget
{
    Q_OBJECT

public:
    explicit WidgetBtnPanel(QWidget *parent = nullptr);
    ~WidgetBtnPanel();
	enum RunState {
		IdleState,
		ResetState,
		ProduceState
	};

protected:
    void  SetState(RunState state);

protected:
    SwitchButton* m_autoModel = nullptr;

public slots:
    void  slotAutoModel(bool checked);

private slots:
    void on_btnReset_clicked();

    void on_btnAutoPause_clicked();

    void on_btnAutoRestore_clicked();

    void on_btnAlarmCleared_clicked();

    void on_btnEstop_clicked();

public slots:
    int event_SetMachineStatus(MachineRunStatus status);

private:
    Ui::WidgetBtnPanel *ui;
};

#endif // WIDGETBTNPANEL_H
