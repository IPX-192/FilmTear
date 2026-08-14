#ifndef WidgetRecipeFilmTear_H
#define WidgetRecipeFilmTear_H

#include <QWidget>
#include "ParamManager.h"

namespace Ui {
class WidgetRecipeFilmTear;
}

class WidgetRecipeFilmTear : public QWidget
{
    Q_OBJECT

public:
    explicit WidgetRecipeFilmTear(QWidget *parent = nullptr);
    ~WidgetRecipeFilmTear();

public slots:
    void  LoadUIParam();
    void  SaveUIParam();
    void  UpdateParamToUI();
    int   LoadRecipeFile();

private slots:
    void  on_btnSave_clicked();
    void on_comboBox_Recipe_currentIndexChanged(const QString &arg1);

protected:
     RecipeFilmTear m_recipeFilmTear;

private:
    Ui::WidgetRecipeFilmTear *ui;
};

#endif // WidgetRecipeFilmTear_H
