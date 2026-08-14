#pragma execution_character_set("utf-8")
#include "WidgetLoad.h"
#include "ui_WidgetLoad.h"
#include <QMessageBox>

WidgetLoad::WidgetLoad(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WidgetLoad)
{
    ui->setupUi(this);
}

WidgetLoad::~WidgetLoad()
{
    delete ui;
}

void WidgetLoad::closeEvent(QCloseEvent *event)
{
    emit sigCloseEvent();
}

void WidgetLoad::SetMaterialInfo(QList<MaterialInfo> infos)
{
    m_materialInfo = infos;
    for(const auto& item: m_materialInfo){
        ui->materialInfoId->addItem(item.id);
    }
}

void WidgetLoad::ShowMsg(QString msg)
{
    QMessageBox::information(this, "提示", msg);
}

void WidgetLoad::on_btnLoad_clicked()
{
    QString type =  QString::number(ui->type->currentIndex() + 1);
    QString batchNumber = ui->batchNumber->text().trimmed();
    if(batchNumber.isEmpty()){
        ShowMsg("物料批次条码为空");
        return;
    }
    QString erp = ui->erp->text().trimmed();
    if(erp.isEmpty()){
        ShowMsg("erp为空");
        return;
    }
    int loadNum = ui->loadNum->value();
    if(loadNum == 0){
        ShowMsg("上料数量为0");
        return;
    }
    QString materialInfoId = ui->materialInfoId->currentText();

    MaterialLoadInfo info;
    info.type = type;
    info.batchNumber = batchNumber;
    info.loadNum = loadNum;
    info.materialInfoId = materialInfoId;
    info.erp = erp;
    info.version = "";
    info.type = type;
    info.historyBatch = ui->historyBatch->text().trimmed();
    bool bLoad = false;
    QString errMsg = MesHttpPost::Instance()->MaterialLoad(info, bLoad);
    if(!errMsg.isEmpty() || !bLoad){
        ShowMsg(QString("上料失败:%1").arg(errMsg));
        return;
    }
    ShowMsg("上料完成");
}

void WidgetLoad::on_materialInfoId_currentTextChanged(const QString &id)
{
    for(const auto& item: m_materialInfo){
        if(item.id == id){
            ui->batchNumber->setText(item.code);
            ui->erp->setText(item.code);
        }
    }
}
