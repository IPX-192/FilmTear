#include "WidgetRecipeDirty.h"
#include "ui_WidgetRecipeDirty.h"
#include <QMessageBox>
#include <QFileDialog>
#include "VisAppBus.h"
#include "VisUIParam.h"

WidgetRecipeDirty::WidgetRecipeDirty(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WidgetRecipeDirty)
{
    ui->setupUi(this);
}

WidgetRecipeDirty::~WidgetRecipeDirty()
{
    delete ui;
}

void WidgetRecipeDirty::LoadUIParam()
{
    RecipeDirty& recipeDirty = GlobalParam->recipeDirty;
    QString filename = recipeDirty.filepath + "Dirty.xml";
    GlobalParam->LoadRecipeList(filename, recipeDirty.listRecipe, ui->comboBox_Recipe);
}

void WidgetRecipeDirty::SaveUIParam()
{
    QString filename = m_recipeDirty.filepath + m_recipeDirty.curRecipe + ".ini";
    VisUIParam::SaveUIToIni(filename, this, &m_recipeDirty);
    if (GlobalParam->recipeDirty.curRecipe == m_recipeDirty.curRecipe){
        GlobalParam->recipeDirty = m_recipeDirty;
    }
}

void WidgetRecipeDirty::UpdateParamToUI()
{
    VisUIParam::UpdateParamToUI(&m_recipeDirty,this);
}

int WidgetRecipeDirty::LoadRecipeFile()
{
    RecipeDirty& recipeDirty = GlobalParam->recipeDirty;
    //curRecipe为空或不在列表时,优先当前产品对应的配方
    QString prefer = GlobalParam->recipeProduct.GetCurProductRecipe("recipeDirty");
    if (recipeDirty.curRecipe.isEmpty() || !recipeDirty.listRecipe.contains(recipeDirty.curRecipe)) {
        int idx = recipeDirty.listRecipe.indexOf(prefer);
        if (idx < 0) idx = 0;
        if (idx < recipeDirty.listRecipe.size())
            recipeDirty.curRecipe = recipeDirty.listRecipe.at(idx);
    }
    QString filename = recipeDirty.filepath + recipeDirty.curRecipe + ".ini";
    VisUIParam::LoadIniToUI(filename, this, &recipeDirty);
    ui->comboBox_Recipe->blockSignals(true);
    int index = recipeDirty.listRecipe.indexOf(recipeDirty.curRecipe);
    if (index < 0) index = recipeDirty.listRecipe.indexOf(prefer);   //当前产品对应配方
    if (index < 0) index = 0;
    ui->comboBox_Recipe->setCurrentIndex(index);
    ui->comboBox_Recipe->blockSignals(false);
    m_recipeDirty = recipeDirty;
    UpdateParamToUI();
    
    return 0;
}

void WidgetRecipeDirty::on_btnSave_clicked()
{
    QString recipeName = ui->lineEdit->text();
    if (recipeName.isEmpty()) {
        QMessageBox::information(this, u8"提示信息", u8"保存的配方名不能为空");
        return;
    }
    if (m_recipeDirty.listRecipe.contains(recipeName)) {
        QMessageBox::information(this, u8"提示信息", u8"保存的配方名不能重复");
        return;
    }
    ui->lineEdit->setText("");
    ui->comboBox_Recipe->addItem(recipeName);
    m_recipeDirty.listRecipe.push_back(recipeName);
    GlobalParam->recipeDirty.listRecipe = m_recipeDirty.listRecipe;
    QString filename = m_recipeDirty.filepath + "Dirty.xml";
    GlobalParam->SaveRecipeList(filename, GlobalParam->recipeDirty.listRecipe);

    filename = m_recipeDirty.filepath + recipeName + ".ini";
    RecipeDirty recipeDirty;
    VisUIParam::SaveUIToIni(filename, this, &recipeDirty);
}

void WidgetRecipeDirty::on_comboBox_Recipe_currentIndexChanged(const QString &arg1)
{
    m_recipeDirty.curRecipe = arg1;
    QString filename = m_recipeDirty.filepath + arg1 + ".ini";
    VisUIParam::LoadIniToUI(filename, this, &m_recipeDirty);
}

void WidgetRecipeDirty::on_btnBrowseOnnx_clicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, u8"选择ONNX模型文件", "",
        u8"ONNX模型文件 (*.onnx);;所有文件 (*.*)");
    if (!filePath.isEmpty()) {
        ui->lineEdit_modelPath->setText(filePath);
    }
}

void WidgetRecipeDirty::on_btnBrowseMtr_clicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, u8"选择MTR模型文件", "",
        u8"MTR模型文件 (*.mtr);;所有文件 (*.*)");
    if (!filePath.isEmpty()) {
        ui->lineEdit_modelPathMtr->setText(filePath);
    }
}
