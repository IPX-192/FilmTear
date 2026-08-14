#ifndef WIDGETRECIPEGRIP_H
#define WIDGETRECIPEGRIP_H

#include <QWidget>
#include "ParamManager.h"

namespace Ui {
class WidgetRecipeGrip;
}

class WidgetRecipeGrip : public QWidget
{
    Q_OBJECT

public:
    explicit WidgetRecipeGrip(QWidget *parent = nullptr);
    ~WidgetRecipeGrip();

public slots:
    void  LoadUIParam();
    void  SaveUIParam();
    void  UpdateParamToUI();
    int   LoadRecipeFile();

protected:
    void  SetGripParam();

private slots:
    void on_btnSave_clicked();

    void on_comboBox_Recipe_currentIndexChanged(const QString &arg1);

private:
    Ui::WidgetRecipeGrip *ui;
    RecipeGrip  m_recipeGrip;
};

#endif // WIDGETRECIPEGRIP_H
