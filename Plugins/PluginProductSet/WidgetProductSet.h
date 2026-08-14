#ifndef WIDGETPRODUCTSET_H
#define WIDGETPRODUCTSET_H

#include <QWidget>
#include "ParamManager.h"

namespace Ui {
class WidgetProductSet;
}
class WidgetRecipeTray;
class WidgetProductSet :public QWidget
{
    Q_OBJECT

public:
    explicit WidgetProductSet(QWidget *parent = nullptr);
    ~WidgetProductSet();

public:
	void  LoadUIParam();
	void  UpdateUIParam();

public slots:
    void slot_InputInfo(tagOutputInfo& inputInfo);
    int  event_LoginUserChange();

private slots:
    void on_btnUpdateUI_clicked();

    void on_btnSaveParam_clicked();

private:
    Ui::WidgetProductSet *ui;
};

#endif // WIDGETPRODUCTSET_H
