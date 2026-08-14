#ifndef WIDGETRECIPEMOTOR_H
#define WIDGETRECIPEMOTOR_H

#include <QWidget>
#include <QVBoxLayout>

namespace Ui {
class WidgetRecipeMotor;
}

class WidgetRecipeMotor : public QWidget
{
    Q_OBJECT

public:
    explicit WidgetRecipeMotor(QWidget *parent = nullptr);
    ~WidgetRecipeMotor();


public slots:
    void  LoadUIParam();
    void  SaveUIParam();
    void  UpdateParamToUI();
    int   LoadRecipeFile();
    int   event_MotorUiAdd();

private slots:
    void on_btnSave_clicked();

    void on_comboBox_Recipe_currentIndexChanged(const QString &arg1);

private:
    Ui::WidgetRecipeMotor *ui;
    QVBoxLayout *m_layout = nullptr;
    QWidget *m_layoutWidget = nullptr;
};

#endif // WIDGETRECIPEMOTOR_H
