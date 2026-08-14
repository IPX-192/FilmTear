#include "TrayRfidForm.h"
#include "ui_TrayRfidForm.h"
#include "VisAppBus.h"
#include "VisMotorManager.h"

#include <QMessageBox>
#include <QInputDialog>
#pragma execution_character_set("utf-8")

TrayRfidForm::TrayRfidForm(QWidget *parent)
    : QWidget(parent), ui(new Ui::TrayRfidForm)
{
    ui->setupUi(this);
    ui->combox_StrbaudRate->addItems({"9600","19200","38400","115200"});
    ui->combox_Strstation->clear();
    ui->combox_Strstation->addItem(u8"上料PCB");
    VisAppBus::subscibeEvent(this, "ScanTrayReady");
}

TrayRfidForm::~TrayRfidForm()
{
    delete ui;
}

QWidget *TrayRfidForm::GetUiWidget()
{
    return this;
}

TrayFunc TrayRfidForm::getCurrentStation() const
{
    return FeedPCB;
}

void TrayRfidForm::on_btnOpenCom_clicked()
{
    QString port = ui->lineEdit_comPort->text().trimmed();
    int baud = ui->combox_StrbaudRate->currentText().toInt();
    if (port.isEmpty() || baud <= 0)
    {
        QMessageBox::warning(this, u8"参数错误", u8"请填写有效的串口与波特率！");
        return;
    }
    int ret = VisAppBus::sendEvent("ConnectModbus", port, baud);
    if (ret == 0){
        QMessageBox::information(this, u8"成功", QString(u8"串口 %1 打开成功").arg(port));
        ui->btnOpenCom->setText(u8"已打开");
        ui->btnCloseCom->setText(u8"关闭");
        m_comOpened = true;
    }
    else
        QMessageBox::critical(this, u8"失败", QString(u8"串口 %1 打开失败！").arg(port));
}

int TrayRfidForm::event_ScanTrayReady()
{
    //托盘扫码枪(RFID)自动连接成功,同步串口按钮状态
    ui->btnOpenCom->setText(u8"已打开");
    ui->btnCloseCom->setText(u8"关闭");
    m_comOpened = true;
    return 0;
}

void TrayRfidForm::on_btnCloseCom_clicked()
{
    VisAppBus::sendEvent("DisconnectedModbus");
    ui->btnOpenCom->setText(u8"打开");
    ui->btnCloseCom->setText(u8"已关闭");
    m_comOpened = false;
}

void TrayRfidForm::on_btnGetCode_clicked()
{
    if (!m_comOpened) {
        QMessageBox::warning(this, u8"提示", u8"请先打开串口！");
        return;
    }
    if (VisMotorToolSpace::VisMotorInstance->IsEmgStop()) {
        QMessageBox::warning(this, u8"提示", u8"设备急停中,无法读取条码,请先清除急停！");
        return;
    }
    TrayFunc station = getCurrentStation();
    QString barCode;
    int ret = VisAppBus::sendEvent("ReadRfidTag", station, barCode);
    if(ret == 0)
    {
        ui->textEditResult->setPlainText(barCode.trimmed());
    }
    else
    {
        ui->textEditResult->setPlainText(u8"读卡失败/未检测到标签");
    }
}

void TrayRfidForm::on_btnWriteCode_clicked()
{
    if (!m_comOpened) {
        QMessageBox::warning(this, u8"提示", u8"请先打开串口！");
        return;
    }
    if (VisMotorToolSpace::VisMotorInstance->IsEmgStop()) {
        QMessageBox::warning(this, u8"提示", u8"设备急停中,无法写入条码,请先清除急停！");
        return;
    }
    TrayFunc station = getCurrentStation();
    QString code = QInputDialog::getText(this, u8"写入托盘条码", u8"请输入待写入条码：");
    if (code.isEmpty())
        return;
    if (code.size() % 2 != 0) {
        QMessageBox::warning(this, u8"提示", u8"条码长度必须是偶数！");
        return;
    }

    int ret = VisAppBus::sendEvent("SetModbusTrayCode", station, code);
    if (ret == 0)
        QMessageBox::information(this, u8"成功", u8"条码写入完成！");
    else
        QMessageBox::critical(this, u8"失败", u8"条码写入失败！");
}
