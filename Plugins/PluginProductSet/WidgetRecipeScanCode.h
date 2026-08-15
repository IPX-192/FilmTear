#ifndef WIDGETRECIPESCANCODE_H
#define WIDGETRECIPESCANCODE_H

#include <QWidget>
#include "ParamManager.h"

namespace Ui {
class WidgetRecipeScanCode;
}

class WidgetRecipeScanCode : public QWidget
{
    Q_OBJECT

public:
    explicit WidgetRecipeScanCode(QWidget *parent = nullptr);
    ~WidgetRecipeScanCode();

public slots:
    void  LoadUIParam();
    void  SaveUIParam();
    void  UpdateParamToUI();
    int   LoadRecipeFile();

private slots:
    void  on_btnSave_clicked();
    void on_comboBox_Recipe_currentIndexChanged(const QString &arg1);

private:
    Ui::WidgetRecipeScanCode *ui;
};

#endif // WIDGETRECIPESCANCODE_H
