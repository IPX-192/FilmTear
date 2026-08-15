#include "WidgetRecipeGrip.h"
#include "ui_WidgetRecipeGrip.h"
#include "VisAppBus.h"
#include "VisUIParam.h"
#include "VisMotorManager.h"
#include <QDir>
#include <QMessageBox>

using namespace VisMotorToolSpace;

WidgetRecipeGrip::WidgetRecipeGrip(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WidgetRecipeGrip)
{
    ui->setupUi(this);
}

WidgetRecipeGrip::~WidgetRecipeGrip()
{
    delete ui;
}

void WidgetRecipeGrip::LoadUIParam()
{
    RecipeGrip& recipeGrip = GlobalParam->recipeGrip;
    QString filename = recipeGrip.filepath + "Grip.xml";
    GlobalParam->LoadRecipeList(filename, m_recipeGrip.listRecipe, ui->comboBox_Recipe);
}

void WidgetRecipeGrip::SaveUIParam()
{
    QString filename = m_recipeGrip.filepath + m_recipeGrip.curRecipe + ".ini";
    VisUIParam::SaveUIToIni(filename, this, &m_recipeGrip);
    if (GlobalParam->recipeGrip.curRecipe == m_recipeGrip.curRecipe)
    {
        VisUIParam::QObjectCopy(&m_recipeGrip, &GlobalParam->recipeGrip);
        SetGripParam();
    }
}

void WidgetRecipeGrip::UpdateParamToUI()
{
    VisUIParam::UpdateParamToUI(&m_recipeGrip,this);
}

int WidgetRecipeGrip::LoadRecipeFile()
{
    RecipeGrip& recipeGrab = GlobalParam->recipeGrip;
    QString filename = recipeGrab.filepath + "GrabParams.xml";
    GlobalParam->LoadRecipeList(filename, recipeGrab.listRecipe, ui->comboBox_Recipe);

    //curRecipe为空或不在列表时,优先当前产品对应的配方
    QString prefer = GlobalParam->recipeProduct.GetCurProductRecipe("recipeGrip");
    if (recipeGrab.curRecipe.isEmpty() || !recipeGrab.listRecipe.contains(recipeGrab.curRecipe)) {
        int idx = recipeGrab.listRecipe.indexOf(prefer);
        if (idx < 0) idx = 0;
        if (idx < recipeGrab.listRecipe.size())
            recipeGrab.curRecipe = recipeGrab.listRecipe.at(idx);
    }

    filename = GlobalParam->recipeGrip.filepath + GlobalParam->recipeGrip.curRecipe + ".ini";
    VisUIParam::LoadIniToUI(filename, this, &GlobalParam->recipeGrip);
    VisUIParam::QObjectCopy(&GlobalParam->recipeGrip, &m_recipeGrip);
    ui->comboBox_Recipe->blockSignals(true);
    int index = m_recipeGrip.listRecipe.indexOf(m_recipeGrip.curRecipe);
    if (index < 0) index = m_recipeGrip.listRecipe.indexOf(prefer);   //当前产品对应配方
    if (index < 0) index = 0;
    ui->comboBox_Recipe->setCurrentIndex(index);
    ui->comboBox_Recipe->blockSignals(false);
    SetGripParam();

    return 0;
}

void WidgetRecipeGrip::SetGripParam()
{
    int grabN=ui->spinBox_grabN->value();
    QStringList listMotorX,listMotorR,listMotorName;
    listMotorX<<MotorHolderGripX<<MotorPCBGripX<<MotorCleanGripX
             <<MotorTurntableFeedGripX<<MotorTurntableBlankGripX<<MotorFilmtearGripX<<MotorTurntableCleanGripX;
    listMotorR<<MotorHolderGripR<<MotorPCBGripR<<MotorCleanGripR
             <<MotorTurntableFeedGripR<<MotorTurntableBlankGripR<<MotorFilmtearGripR<<MotorTurntableCleanGripR;
    listMotorName<<u8"上料壳体"<<u8"上料PCB"<<u8"流线PCB清洗"<<u8"转盘上料"<<u8"转盘下料"<<u8"撕膜"<<u8"转盘清洗";

    for(int i=0;i<listMotorX.size();i++){
        bool flagOk=VisMotorInstance->SetStrength(listMotorX.at(i), grabN);
        if(!flagOk){
            //ShowSystemLog(Log_Error, QString(u8"%1夹爪夹持力设置失败").arg(listMotorName.at(i)));
        }
    }
}

void WidgetRecipeGrip::on_btnSave_clicked()
{
    QString recipeName = ui->lineEdit->text();
    if (recipeName.isEmpty()) {
        QMessageBox::information(this, u8"提示信息", u8"保存的配方名不能为空");
        return;
    }
    if (m_recipeGrip.listRecipe.contains(recipeName)) {
        QMessageBox::information(this, u8"提示信息", u8"保存的配方名不能重复");
        return;
    }
    ui->lineEdit->setText("");
    ui->comboBox_Recipe->addItem(recipeName);
    m_recipeGrip.listRecipe.push_back(recipeName);
    GlobalParam->recipeGrip.listRecipe = m_recipeGrip.listRecipe;
    QString filename = m_recipeGrip.filepath + "GrabParams.xml";
    GlobalParam->SaveRecipeList(filename, GlobalParam->recipeGrip.listRecipe);

    filename = m_recipeGrip.filepath + recipeName + ".ini";
    RecipeGrip recipeGrab;
    VisUIParam::SaveUIToIni(filename, this, &recipeGrab);
}

void WidgetRecipeGrip::on_comboBox_Recipe_currentIndexChanged(const QString &arg1)
{
    m_recipeGrip.curRecipe = arg1;
    QString filename = m_recipeGrip.filepath + arg1 + ".ini";
    VisUIParam::LoadIniToUI(filename, this, &m_recipeGrip);
}
