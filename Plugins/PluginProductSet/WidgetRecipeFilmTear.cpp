#include "WidgetRecipeFilmTear.h"
#include "ui_WidgetRecipeFilmTear.h"
#include <QMessageBox>
#include "VisAppBus.h"
#include "VisUIParam.h"

WidgetRecipeFilmTear::WidgetRecipeFilmTear(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WidgetRecipeFilmTear)
{
    ui->setupUi(this);
}

WidgetRecipeFilmTear::~WidgetRecipeFilmTear()
{
    delete ui;
}

void WidgetRecipeFilmTear::LoadUIParam()
{
    RecipeFilmTear& recipeFilmTear = GlobalParam->recipeFilmTear;
    QString filename = recipeFilmTear.filepath + "FilmTear.xml";
    GlobalParam->LoadRecipeList(filename, recipeFilmTear.listRecipe, ui->comboBox_Recipe);
    //LoadRecipeFile();
}

void WidgetRecipeFilmTear::SaveUIParam()
{
    QString filename = m_recipeFilmTear.filepath + m_recipeFilmTear.curRecipe + ".ini";
    VisUIParam::SaveUIToIni(filename, this, &m_recipeFilmTear);
    if (GlobalParam->recipeFilmTear.curRecipe == m_recipeFilmTear.curRecipe){
        GlobalParam->recipeFilmTear = m_recipeFilmTear;
    }
}

void WidgetRecipeFilmTear::UpdateParamToUI()
{
    VisUIParam::UpdateParamToUI(&m_recipeFilmTear,this);
}

int WidgetRecipeFilmTear::LoadRecipeFile()
{
    RecipeFilmTear& recipeFilmTear = GlobalParam->recipeFilmTear;
    QString filename = recipeFilmTear.filepath + recipeFilmTear.curRecipe + ".ini";
    VisUIParam::LoadIniToUI(filename, this, &recipeFilmTear);
    ui->comboBox_Recipe->blockSignals(true);
    int index = recipeFilmTear.listRecipe.indexOf(recipeFilmTear.curRecipe);
    ui->comboBox_Recipe->setCurrentIndex(index);
    ui->comboBox_Recipe->blockSignals(false);
    m_recipeFilmTear = recipeFilmTear;
    UpdateParamToUI();
    
    return 0;
}

void WidgetRecipeFilmTear::on_btnSave_clicked()
{
    QString recipeName = ui->lineEdit->text();
    if (recipeName.isEmpty()) {
        QMessageBox::information(this, u8"提示信息", u8"保存的配方名不能为空");
        return;
    }
    if (m_recipeFilmTear.listRecipe.contains(recipeName)) {
        QMessageBox::information(this, u8"提示信息", u8"保存的配方名不能重复");
        return;
    }
    ui->lineEdit->setText("");
    ui->comboBox_Recipe->addItem(recipeName);
    m_recipeFilmTear.listRecipe.push_back(recipeName);
    GlobalParam->recipeFilmTear.listRecipe = m_recipeFilmTear.listRecipe;
    QString filename = m_recipeFilmTear.filepath + "FilmTear.xml";
    GlobalParam->SaveRecipeList(filename, GlobalParam->recipeFilmTear.listRecipe);

    filename = m_recipeFilmTear.filepath + recipeName + ".ini";
    RecipeFilmTear recipeFilmTear;
    VisUIParam::SaveUIToIni(filename, this, &recipeFilmTear);
}

void WidgetRecipeFilmTear::on_comboBox_Recipe_currentIndexChanged(const QString &arg1)
{
    m_recipeFilmTear.curRecipe = arg1;
    QString filename = m_recipeFilmTear.filepath + arg1 + ".ini";
    VisUIParam::LoadIniToUI(filename, this, &m_recipeFilmTear);
}
