#ifndef WIDGETMOTORCTRL_H
#define WIDGETMOTORCTRL_H

#include <QWidget>
#include "Singleton.h"
#include <QVBoxLayout>

namespace Ui {
class WidgetMotorCtrl;
}

class WidgetMotorCtrl : public QWidget
{
    Q_OBJECT
    SINGLETON_DECL(WidgetMotorCtrl)
public:
    explicit WidgetMotorCtrl(QWidget *parent = nullptr);
    ~WidgetMotorCtrl();

public:
    void InitMotor();

protected:
    void showEvent(QShowEvent *event);
    void closeEvent(QCloseEvent *event);

private:
    Ui::WidgetMotorCtrl *ui;
    QWidget *m_widgetMotor = nullptr;
    QWidget *m_widgetIoDebug = nullptr;
    QVBoxLayout *m_layout = nullptr;
};

#endif // WIDGETMOTORCTRL_H
