#ifndef WIDGETSTATUSBAR_H
#define WIDGETSTATUSBAR_H

#include <QWidget>
#include <QTimer>

namespace Ui {
class WidgetStatusBar;
}

class WidgetStatusBar : public QWidget
{
    Q_OBJECT

public:
    explicit WidgetStatusBar(QWidget *parent = nullptr);
    ~WidgetStatusBar();

protected:
    QTimer*m_timer=nullptr;

public slots:
    void  slotTimeout();

private:
    Ui::WidgetStatusBar *ui;
};

#endif // WIDGETSTATUSBAR_H
