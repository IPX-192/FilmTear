#include "WidgetRecipePlatform.h"
#include "ui_WidgetRecipePlatform.h"
#include <QMessageBox>
#include "ItemDelegate.h"
#include "ParamManager.h"
#include "VisAppBus.h"
#include "VisUIParam.h"

WidgetRecipePlatform::WidgetRecipePlatform(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WidgetRecipePlatform)
{
    ui->setupUi(this);
    InitTable();
}

WidgetRecipePlatform::~WidgetRecipePlatform()
{
    delete ui;
}

void WidgetRecipePlatform::InitTable()
{
    //ui->tableView->setProperty("video",true);
    ui->tableView->verticalHeader()->setVisible(false);
    ui->tableView->horizontalHeader()->setVisible(false);

    ui->tableView->horizontalHeader()->setVisible(true);
    ui->tableView->verticalHeader()->setDefaultSectionSize(35);
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);	//设置选择一行
    //tableView->horizontalHeader()->setStretchLastSection(true);		//设置最后一列自适应
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);  //自适应列宽
    ui->tableView->verticalHeader()->setDefaultSectionSize(25);			//设置默认行高

    QStringList headerList;
    for(int i=0;i<GlobalParam->recipeProduct.vecRecipeNode.size();i++){
         headerList<<GlobalParam->recipeProduct.vecRecipeNode.at(i).first;
    }
    m_pModel = new QStandardItemModel(this);
    m_pModel->setHorizontalHeaderLabels(headerList);
    ui->tableView->setModel(m_pModel);
	ItemDelegate* itemDelegate = new ItemDelegate(this);
    ui->tableView->setItemDelegate(itemDelegate);
}

void WidgetRecipePlatform::LoadUIParam()
{
    bool bRet = GlobalParam->LoadRecipeProduct();
	ShowSystemLog(bRet ? Log_Info : Log_Error, QString(u8"产品配方文件加载%1！").arg(bRet ? u8"成功" : u8"失败"));
	UpdateParamToUI();
}

void WidgetRecipePlatform::SaveUIParam()
{
    QVector<QVector<QPair<QString,QString>>> listRecipe;
    for (int i = 0; i < m_pModel->rowCount(); i++) {
        QVector<QPair<QString,QString>>vecRecipe;
        for(int j=0;j<m_pModel->columnCount();j++){
            QString key=m_pModel->headerData(j,Qt::Horizontal).toString();
            QString value=m_pModel->data(m_pModel->index(i, j)).toString();
            vecRecipe.push_back(QPair<QString,QString>(key,value));
        }
        listRecipe.push_back(vecRecipe);
    }
    GlobalParam->recipeProduct.vecRecipeDetail = listRecipe;
	bool bRet = GlobalParam->SaveRecipeProduct();
	ShowSystemLog(bRet ? Log_Info : Log_Error, QString(u8"产品配方文件保存%1！").arg(bRet ? u8"成功" : u8"失败"));
	QMessageBox::information(this, u8"提示信息", u8"保存参数成功");
	tagOutputInfo outInfo;
	outInfo._type = INFT_ProductChange;
	emit GlobalParam->frameCore->sig_OutputInfo(outInfo);
}

void WidgetRecipePlatform::UpdateParamToUI()
{
    QVector<MatrixSetting>& listPlatformMatrix = GlobalParam->recipeProduct.listPlatformMatrix;
    QVector<QVector<QPair<QString,QString>>>& listOldRecipe = GlobalParam->recipeProduct.vecRecipeDetail;

    QVector<QVector<QPair<QString,QString>>> listNewRecipe;
    QStringList listProductName;
    for (int i = 0; i < listPlatformMatrix.size(); i++) {
        if(listProductName.contains(listPlatformMatrix[i].productName))continue;
		listProductName << listPlatformMatrix[i].productName;
        QVector<QPair<QString,QString>>vecRecipe;
        vecRecipe.push_back(QPair<QString, QString>(u8"型号名称", listPlatformMatrix[i].productName));
        for (int j = 0; j < listOldRecipe.size(); j++) {
            for(int k=0;k<listOldRecipe[j].size();k++){
                if (listOldRecipe[j][k].second==listPlatformMatrix[i].productName) {
                    vecRecipe = listOldRecipe[j];
                    break;
                }
            }
        }
        listNewRecipe.push_back(vecRecipe);
    }
	m_pModel->setRowCount(listNewRecipe.size());
	for (int i = 0; i < listNewRecipe.size(); i++) {
        for(int  j=0;j<listNewRecipe[i].size();j++){
            m_pModel->setData(m_pModel->index(i, j), Qt::AlignCenter, Qt::TextAlignmentRole);
            m_pModel->setData(m_pModel->index(i, j), listNewRecipe[i][j].second);
            if(j==0)continue;
            m_pModel->setData(m_pModel->index(i, j), ComboBoxDelegate, ItemType);
        }
	}
    GlobalParam->recipeProduct.vecRecipeDetail = listNewRecipe;
}

void WidgetRecipePlatform::slotAddPlatfrom()
{
	UpdateParamToUI();
	bool bRet = GlobalParam->SaveRecipeProduct();
	ShowSystemLog(bRet ? Log_Info : Log_Error, QString(u8"产品配方文件保存%1！").arg(bRet ? u8"成功" : u8"失败"));
}


