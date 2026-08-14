#ifndef WIDGETRECIPEDIRTY_H
#define WIDGETRECIPEDIRTY_H

#include <QWidget>
#include "ParamManager.h"

namespace Ui {
class WidgetRecipeDirty;
}

class WidgetRecipeDirty : public QWidget
{
    Q_OBJECT

public:
    explicit WidgetRecipeDirty(QWidget *parent = nullptr);
    ~WidgetRecipeDirty();

public slots:
    void  LoadUIParam();
    void  SaveUIParam();
    void  UpdateParamToUI();
    int   LoadRecipeFile();

private slots:
    void  on_btnSave_clicked();
    void  on_comboBox_Recipe_currentIndexChanged(const QString &arg1);
    void  on_btnBrowseOnnx_clicked();
    void  on_btnBrowseMtr_clicked();

protected:
    RecipeDirty m_recipeDirty;

private:
    Ui::WidgetRecipeDirty *ui;
};

#endif // WIDGETRECIPEDIRTY_H
