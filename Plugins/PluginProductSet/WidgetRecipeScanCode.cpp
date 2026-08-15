#include "WidgetRecipeScanCode.h"
#include "ui_WidgetRecipeScanCode.h"
#include <QMessageBox>
#include <QFile>
#include "VisUIParam.h"

WidgetRecipeScanCode::WidgetRecipeScanCode(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WidgetRecipeScanCode)
{
    ui->setupUi(this);
}

WidgetRecipeScanCode::~WidgetRecipeScanCode()
{
    delete ui;
}

void WidgetRecipeScanCode::LoadUIParam()
{
    RecipeScanCode& recipeScanCode = GlobalParam->recipeScanCode;
    QString filename = recipeScanCode.filepath + "ScanCode.xml";
    GlobalParam->LoadRecipeList(filename, recipeScanCode.listRecipe, ui->comboBox_Recipe);
    LoadRecipeFile();
}

void WidgetRecipeScanCode::SaveUIParam()
{
    RecipeScanCode& recipeScanCode = GlobalParam->recipeScanCode;
    QString filename = recipeScanCode.filepath + recipeScanCode.curRecipe + ".ini";
    VisUIParam::SaveUIToIni(filename, this, &recipeScanCode);
}

void WidgetRecipeScanCode::UpdateParamToUI()
{
    RecipeScanCode& recipeScanCode = GlobalParam->recipeScanCode;
    VisUIParam::UpdateParamToUI(&recipeScanCode, this);
}

int WidgetRecipeScanCode::LoadRecipeFile()
{
    RecipeScanCode& recipeScanCode = GlobalParam->recipeScanCode;
    //curRecipe为空或不在列表时,优先当前产品对应的配方
    QString prefer = GlobalParam->recipeProduct.GetCurProductRecipe("recipeScanCode");
    if (recipeScanCode.curRecipe.isEmpty() || !recipeScanCode.listRecipe.contains(recipeScanCode.curRecipe)) {
        int idx = recipeScanCode.listRecipe.indexOf(prefer);
        if (idx < 0) idx = 0;
        if (idx < recipeScanCode.listRecipe.size())
            recipeScanCode.curRecipe = recipeScanCode.listRecipe.at(idx);
    }
    QString filename = recipeScanCode.filepath + recipeScanCode.curRecipe + ".ini";
    VisUIParam::LoadIniToUI(filename, this, &recipeScanCode);
    ui->comboBox_Recipe->blockSignals(true);
    int index = recipeScanCode.listRecipe.indexOf(recipeScanCode.curRecipe);
    if (index < 0) index = recipeScanCode.listRecipe.indexOf(prefer);   //当前产品对应配方
    if (index < 0) index = 0;
    ui->comboBox_Recipe->setCurrentIndex(index);
    ui->comboBox_Recipe->blockSignals(false);

    return 0;
}

void WidgetRecipeScanCode::on_btnSave_clicked()
{
    QString recipeName = ui->lineEdit->text();
    if (recipeName.isEmpty()) {
        QMessageBox::information(this, u8"提示信息", u8"保存的配方名不能为空");
        return;
    }
    RecipeScanCode& recipeScanCode = GlobalParam->recipeScanCode;
    if (recipeScanCode.listRecipe.contains(recipeName)) {
        QMessageBox::information(this, u8"提示信息", u8"保存的配方名不能重复");
        return;
    }
    //先把当前界面值保存到当前配方文件,再复制为新配方文件
    QString curFileName;
    if (!recipeScanCode.curRecipe.isEmpty())
        curFileName = recipeScanCode.filepath + recipeScanCode.curRecipe + ".ini";
    else
        curFileName = recipeScanCode.filepath + "EQ4-100.ini";
    VisUIParam::SaveUIToIni(curFileName, this, &recipeScanCode);
    QString newFileName = recipeScanCode.filepath + recipeName + ".ini";
    QFile::copy(curFileName, newFileName);

    ui->lineEdit->setText("");
    ui->comboBox_Recipe->blockSignals(true);
    ui->comboBox_Recipe->addItem(recipeName);
    ui->comboBox_Recipe->blockSignals(false);
    recipeScanCode.listRecipe.push_back(recipeName);

    QString filename = recipeScanCode.filepath + "ScanCode.xml";
    GlobalParam->SaveRecipeList(filename, recipeScanCode.listRecipe);
}

void WidgetRecipeScanCode::on_comboBox_Recipe_currentIndexChanged(const QString &arg1)
{
    RecipeScanCode& recipeScanCode = GlobalParam->recipeScanCode;
    recipeScanCode.curRecipe = arg1;
    QString filename = recipeScanCode.filepath + arg1 + ".ini";
    VisUIParam::LoadIniToUI(filename, this, &recipeScanCode);
}
