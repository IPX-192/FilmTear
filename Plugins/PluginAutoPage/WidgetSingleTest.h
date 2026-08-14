#ifndef WidgetSingleTest_H
#define WidgetSingleTest_H

#include <QWidget>

namespace Ui {
class WidgetSingleTest;
}

class WidgetSingleTest : public QWidget
{
    Q_OBJECT

public:
    explicit WidgetSingleTest(QWidget *parent = nullptr);
    ~WidgetSingleTest();

public slots:
    int  event_LoginUserChange();
    int  event_UiAutoMode(bool flag);

private:
    void InitSlot();

private:
    Ui::WidgetSingleTest *ui;
};

#endif // WidgetSingleTest_H
