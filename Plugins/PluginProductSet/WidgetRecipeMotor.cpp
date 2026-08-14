#include "WidgetRecipeMotor.h"
#include "ui_WidgetRecipeMotor.h"
#include <QMessageBox>
#include "VisMotorTool.h"
#include "VisMotorToolData.h"
#include "VisMotorManager.h"
#include "ParamManager.h"

using namespace VisMotorToolSpace;

WidgetRecipeMotor::WidgetRecipeMotor(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WidgetRecipeMotor)
{
    ui->setupUi(this);
    VisAppBus::subscibeEvent(this, "MotorUiAdd");

    // 创建垂直布局
    m_layout = new QVBoxLayout(this);
    m_layoutWidget = new QWidget(this);
    event_MotorUiAdd();

    ui->groupBox->layout()->addWidget(m_layoutWidget);
}

WidgetRecipeMotor::~WidgetRecipeMotor()
{
    delete ui;
}

void WidgetRecipeMotor::LoadUIParam()
{
	RecipeMotor& recipeMotor = GlobalParam->recipeMotor;
	QString filename = recipeMotor.filepath + "MotorList.xml";
	GlobalParam->LoadRecipeList(filename, recipeMotor.listRecipe, ui->comboBox_Recipe);
	//LoadRecipeFile();
}

void WidgetRecipeMotor::SaveUIParam()
{

}

void WidgetRecipeMotor::UpdateParamToUI()
{

}

int WidgetRecipeMotor::LoadRecipeFile()
{
	RecipeMotor& recipeMotor = GlobalParam->recipeMotor;
	QString filename = recipeMotor.filepath + recipeMotor.curRecipe + ".xml";
	bool bRet = VisMotorToolIns->SetPointFile(filename);
	ShowSystemLog(bRet ? Log_Info : Log_Error, QString(u8"伺服配方文件加载%1！").arg(bRet ? u8"成功" : u8"失败"));
	ui->comboBox_Recipe->blockSignals(true);
	int index = recipeMotor.listRecipe.indexOf(recipeMotor.curRecipe);
	ui->comboBox_Recipe->setCurrentIndex(index);
	ui->comboBox_Recipe->blockSignals(false);

    return 0;
}

int WidgetRecipeMotor::event_MotorUiAdd()
{
    VisMotorToolIns->GetWidget(VisMotorTool::F_MotorDebugForm)->setMinimumHeight(0);
    m_layout->addWidget(VisMotorToolIns->GetWidget(VisMotorTool::F_MotorDebugForm));
    m_layout->addWidget(VisMotorToolIns->GetWidget(VisMotorTool::F_PointDebugForm));
    m_layout->setStretch(0, 2);
    m_layout->setStretch(1, 1);
    m_layoutWidget->setLayout(m_layout);
    return 0;
}

void WidgetRecipeMotor::on_btnSave_clicked()
{
	QString recipeName = ui->lineEdit->text();
	if (recipeName.isEmpty()) {
		QMessageBox::information(this, u8"提示信息", u8"保存的配方名不能为空");
		return;
	}
	RecipeMotor& recipeMotor = GlobalParam->recipeMotor;
	if (recipeMotor.listRecipe.contains(recipeName)) {
		QMessageBox::information(this, u8"提示信息", u8"保存的配方名不能重复");
		return;
	}
	ui->lineEdit->setText("");
	ui->comboBox_Recipe->blockSignals(true);
	ui->comboBox_Recipe->addItem(recipeName);
	ui->comboBox_Recipe->blockSignals(false);
	recipeMotor.listRecipe.push_back(recipeName);
	QString curFileName;
	if (!recipeMotor.curRecipe.isEmpty())
		curFileName = recipeMotor.filepath + recipeMotor.curRecipe + ".xml";
	else
		curFileName = GlobalParam->recipeProduct.productPath + "defaultMotor.xml";
	QString newFileName = recipeMotor.filepath + recipeName + ".xml";
	QFile::copy(curFileName, newFileName);

	QString  filename = recipeMotor.filepath + "MotorList.xml";
	GlobalParam->SaveRecipeList(filename, recipeMotor.listRecipe);
}

void WidgetRecipeMotor::on_comboBox_Recipe_currentIndexChanged(const QString &arg1)
{
	QString filename = GlobalParam->recipeMotor.filepath + arg1 + ".xml";
	bool bRet = VisMotorToolIns->SetPointFile(filename);
	ShowSystemLog(bRet ? Log_Info : Log_Error, QString(u8"伺服配方文件加载%1！").arg(bRet ? u8"成功" : u8"失败"));
}
