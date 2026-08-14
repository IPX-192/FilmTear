#include "WidgetProductChange.h"
#include "ui_WidgetProductChange.h"
#include "ParamManager.h"
#include <QDir>
#include <QString>
#include <QPushButton>
#include <QMessageBox>
#include <QTextCodec>
#include <QTextStream>

WidgetProductChange::WidgetProductChange(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WidgetProductChange)
{
    ui->setupUi(this);
    connect(GlobalParam->frameCore, &QVisF_Interface::sig_OutputInfo, this, &WidgetProductChange::slot_InputInfo);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableWidget->verticalHeader()->setVisible(false);
    // 不可编辑
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    // 无焦点
    ui->tableWidget->setFocusPolicy(Qt::NoFocus);
    // 禁用选择
    ui->tableWidget->setSelectionMode(QAbstractItemView::NoSelection);

    ReadChangeInfo();
}

WidgetProductChange::~WidgetProductChange()
{
    delete ui;
}

void WidgetProductChange::slot_InputInfo(tagOutputInfo &inputInfo)
{
    if (inputInfo._type == INFT_ProductChange) {
        ui->comboBox_SelectProduct->blockSignals(true);
        //产品型号发生变化，更新UI
        ui->comboBox_SelectProduct->clear();
        QVector<MatrixSetting>& listPlatformMatrix=GlobalParam->recipeProduct.listPlatformMatrix;  //平台列表映射
        for (int i=0;i<listPlatformMatrix.size();i++) {
            ui->comboBox_SelectProduct->addItem(listPlatformMatrix[i].productShow,listPlatformMatrix[i].productName);
            if(listPlatformMatrix[i].productShow==GlobalParam->recipeProduct.curProduct)
                GlobalParam->recipeProduct.curMatrix=&listPlatformMatrix[i];
        }
        if (listPlatformMatrix.size() > 0) {
            ui->comboBox_SelectProduct->setCurrentText(GlobalParam->recipeProduct.curProduct);
        }
		if (GlobalParam->recipeProduct.curMatrix != nullptr)
            GlobalParam->frameCore->systemName = QString(u8"软件版本1.0.0  机种型号:%1").arg(GlobalParam->recipeProduct.curMatrix->productShow);
        ui->comboBox_SelectProduct->blockSignals(false);
    }
}

void WidgetProductChange::on_pushButton_clicked()
{
	if (GlobalParam->frameCore->curUserInfo.authority == OPERATOR) {
		QMessageBox::warning(this, QString(u8"警告"), QString(u8"当前用户无权限"));
		return;
	}
	if (GlobalParam->autoRunning) {
		QMessageBox::warning(this, QString(u8"警告"), QString(u8"正在自动运行中，无法切换产品型号"));
		return;
	}
    QString curProductName = ui->comboBox_SelectProduct->currentData().toString();


    QVector<MatrixSetting>& listPlatformMatrix = GlobalParam->recipeProduct.listPlatformMatrix;
    for(int i=0;i<listPlatformMatrix.size();i++){
        if(listPlatformMatrix[i].productName == curProductName){
            GlobalParam->recipeProduct.curMatrix = &listPlatformMatrix[i];
            GlobalParam->recipeProduct.UpdateRecipe();
            break;
        }
    }

    GlobalParam->recipeProduct.curProduct = ui->comboBox_SelectProduct->currentText();

    tagOutputInfo outInfo;
    outInfo._type = INFT_ProductChange;
    this->blockSignals(true);
    emit GlobalParam->frameCore->sig_OutputInfo(outInfo);
    this->blockSignals(false);

    GlobalParam->SaveRecipeProduct();

    SaveChangeInfo();
}

void WidgetProductChange::SaveChangeInfo()
{
    int row = ui->tableWidget->rowCount();
    if (row < 100)
    {
        ui->tableWidget->insertRow(row);
		for (int j = 0; j < ui->tableWidget->columnCount(); ++j)
		{
			QTableWidgetItem *item = new QTableWidgetItem();
			ui->tableWidget->setItem(row, j, item);
		}
    }
	else
	{
		row = 99;
	}
    QString text = QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss");
    ui->tableWidget->item(row, 0)->setText(QString::number(row + 1));
    ui->tableWidget->item(row, 1)->setText(u8"高清14线");
    ui->tableWidget->item(row, 2)->setText(u8"内参标定");
    ui->tableWidget->item(row, 3)->setText(GlobalParam->recipeProduct.curProduct);
    ui->tableWidget->item(row, 4)->setText(text);
    ui->tableWidget->item(row, 5)->setText("TRUE");

    QString filePath = GlobalParam->frameCore->applicationDirPath + "/Config/ProductChange.csv";
    QFile file(filePath);
	if (file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append))
	{
		QTextStream out(&file);
		int column = ui->tableWidget->columnCount();
		// 写入每一行的数据
        out << ++m_csvCount << ",";
        for (int j = 1; j < column; ++j)
		{
			QTableWidgetItem *item = ui->tableWidget->item(row, j);
			QString data = item ? item->text() : QString();
			out << data;
			if (j < column - 1)
				out << ",";
		}
		out << "\n";

		file.close();
	}
	if (row == 99)
	{
		ReadChangeInfo();
	}
}

void WidgetProductChange::ReadChangeInfo()
{
    QString filePath = GlobalParam->frameCore->applicationDirPath + "/Config/ProductChange.csv";
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    QTextStream in(&file);
    QList<QList<QString>> last100Rows;
    QStringList currentRow;
    bool flag = false;
    if (m_csvCount == 0)
        flag = true;
    while (!in.atEnd()) {
        if (flag)
            m_csvCount++;
        currentRow = in.readLine().split(',');

        // 确保每一行的长度与表格的列数匹配
        if (currentRow.size() != ui->tableWidget->columnCount()) {
            break;
        }

        last100Rows.append(currentRow);

        // 维持最后 100 行
        if (last100Rows.size() > 100) {
            last100Rows.removeFirst();
        }
    }

    file.close();

    // 清空表格
    ui->tableWidget->clearContents();
    ui->tableWidget->setRowCount(last100Rows.size());
    ui->tableWidget->setColumnCount(last100Rows.isEmpty() ? 0 : last100Rows.first().size());

    // 填充表格
    for (int i = 0; i < last100Rows.size(); ++i)
    {
        for (int j = 0; j < last100Rows[i].size(); ++j)
        {
            QTableWidgetItem *item = new QTableWidgetItem(last100Rows[i][j]);
            if (j == 0)
                item->setText(QString::number(i + 1));
            ui->tableWidget->setItem(i, j, item);
        }
    }
}
