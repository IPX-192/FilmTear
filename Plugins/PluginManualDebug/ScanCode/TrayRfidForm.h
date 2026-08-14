#ifndef TRAYRFIDFORM_H
#define TRAYRFIDFORM_H

#include <QWidget>
#include "ParamDef.h"

namespace Ui {
class TrayRfidForm;
}

class TrayRfidForm : public QWidget
{
    Q_OBJECT
public:
    explicit TrayRfidForm(QWidget *parent = nullptr);
    ~TrayRfidForm() override;

public slots:
    int  event_ScanTrayReady();   //托盘扫码枪(RFID)连接成功通知,更新串口按钮状态

private slots:
    void on_btnOpenCom_clicked();
    void on_btnCloseCom_clicked();
    void on_btnGetCode_clicked();
    void on_btnWriteCode_clicked();

private:
    Ui::TrayRfidForm *ui;
    TrayFunc getCurrentStation() const;
    bool   m_comOpened = false;
public:
    QWidget* GetUiWidget();
};

#endif // TRAYRFIDFORM_H
