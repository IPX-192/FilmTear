#ifndef SCANCODEFORM_H
#define SCANCODEFORM_H

#include <QWidget>
#include <QSharedPointer>
#include "ParamManager.h"
namespace Ui {
class ScanCodeForm;
}

class ScanCodeForm : public QWidget
{
    Q_OBJECT

public:
    explicit ScanCodeForm(QWidget *parent = nullptr);
    ~ScanCodeForm();

    void SetName(QString chName);

private slots:

    void on_PushButton_Connect_clicked();

    void on_PushButton_DisConnect_clicked();

    void on_PushButton_ScanCode_clicked();

private:


private:
    Ui::ScanCodeForm *ui;
    QString m_chName = "";
};

#endif // SCANCODEFORM_H
