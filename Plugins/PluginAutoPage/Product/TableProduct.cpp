#include "TableProduct.h"
#include "ui_TableProduct.h"
#include <QPushButton>
#include <QStyledItemDelegate>
#include <QMessageBox>

class CenterDelegate : public QStyledItemDelegate
{
public:
	explicit CenterDelegate(QObject* parent = nullptr) : QStyledItemDelegate(parent) {}

	// 重写 initStyleOption 来修改对齐方式
	void initStyleOption(QStyleOptionViewItem* option, const QModelIndex& index) const override
	{
		// 先调用父类实现，加载默认样式
		QStyledItemDelegate::initStyleOption(option, index);
		// 强制修改为居中
		option->displayAlignment = Qt::AlignCenter;
	}
};

TableProduct::TableProduct(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TableProduct)
{
    ui->setupUi(this);
    InitTable();
}

TableProduct::~TableProduct()
{
    delete ui;
}

void TableProduct::UpdateTable(StationInfo* info)
{
	QVector<StationInfo* >vecStationInfo;
	vecStationInfo << info->ptrTotal << info;
	for (int i = 0; i < vecStationInfo.size(); i++) {
		StationInfo* curInfo = vecStationInfo.at(i);
		m_pModel->setData(m_pModel->index(0, i + 1), curInfo->numTotal);
		m_pModel->setData(m_pModel->index(1, i + 1), curInfo->numOK);
		m_pModel->setData(m_pModel->index(2, i + 1), curInfo->numNG);
		if (curInfo->numTotal != 0)
			m_pModel->setData(m_pModel->index(3, i + 1), QString::asprintf("%.2f%", curInfo->numOK * 100.0 / curInfo->numTotal));
		else
			m_pModel->setData(m_pModel->index(3, i + 1), QString("0.00%"));
		m_pModel->setData(m_pModel->index(4, i + 1), QString::asprintf("%.2f", curInfo->timeCT));
	}
}

void TableProduct::InitTable()
{
    ui->tableView->verticalHeader()->setVisible(false);
    ui->tableView->horizontalHeader()->setVisible(false);
    ui->tableView->horizontalHeader()->setVisible(true);
    ui->tableView->verticalHeader()->setDefaultSectionSize(40);
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);	//设置选择一行
    //tableView->horizontalHeader()->setStretchLastSection(true);		//设置最后一列自适应
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);  //自适应列宽
    ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);	//设置不可编辑
    QStringList headerList;
    headerList<< u8"项次" << u8"汇总"<<u8"工位状态";
    m_pModel = new QStandardItemModel(this);
    m_pModel->setHorizontalHeaderLabels(headerList);
    ui->tableView->setItemDelegate(new CenterDelegate(this));
    ui->tableView->setModel(m_pModel);
    QStringList headerV;
    headerV<<u8"投入数"<<u8"良品数"<<u8"不良数"<<u8"良率"<<u8"CT" <<u8"";
	m_pModel->setRowCount(headerV.size());
	for (int i = 0; i < headerV.size(); i++) {
		m_pModel->setData(m_pModel->index(i, 0), headerV.at(i));
		m_pModel->setData(m_pModel->index(i, 1), 0);
		m_pModel->setData(m_pModel->index(i, 2), 0);
	}
    QPushButton* btn1 = new QPushButton(u8"清零",ui->tableView);
	btn1->setProperty("col", 1);
    ui->tableView->setIndexWidget(m_pModel->index(5, 1), btn1);
	QPushButton* btn2 = new QPushButton(u8"清零",ui->tableView);
	btn2->setProperty("col", 2);
	ui->tableView->setIndexWidget(m_pModel->index(5, 2), btn2);
	connect(btn1, &QPushButton::clicked, this, &TableProduct::slotClearBtn);
	connect(btn2, &QPushButton::clicked, this, &TableProduct::slotClearBtn);
}

void TableProduct::slotClearBtn()
{
	int ret = QMessageBox::information(this, u8"警告",
		QString(u8"是否清空数据？"), QMessageBox::StandardButton::Cancel | QMessageBox::StandardButton::Ok);
	if (ret == QMessageBox::StandardButton::Cancel)return;
	
	QPushButton* btn = qobject_cast<QPushButton*>(sender());
	int index = btn->property("col").toInt();
	emit sigClear(index == 1);
}
