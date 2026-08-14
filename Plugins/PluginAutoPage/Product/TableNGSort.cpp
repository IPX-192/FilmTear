#include "TableNGSort.h"
#include "ui_TableNGSort.h"
#include <QStyledItemDelegate>

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

TableNGSort::TableNGSort(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TableNGSort)
{
    ui->setupUi(this);
    InitTable();
}

TableNGSort::~TableNGSort()
{
    delete ui;
}

void TableNGSort::InitTable()
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
    headerList<< u8"项次" << u8"不良项目"<<u8"不良数量"<<u8"不良率";
    m_pModel = new QStandardItemModel(this);
    m_pModel->setHorizontalHeaderLabels(headerList);
    ui->tableView->setItemDelegate(new CenterDelegate(this));
    ui->tableView->setModel(m_pModel);
    
    QStringList headerV;
    for (int i = 0; i < m_rowsNG; i++) {
        headerV << QString(u8"Top%1").arg(i + 1);
    }
    m_pModel->setRowCount(headerV.size());
    for (int i = 0; i < headerV.size(); i++) {
		m_pModel->setData(m_pModel->index(i, 0), headerV.at(i));
        m_pModel->setData(m_pModel->index(i, 1), 0);
        m_pModel->setData(m_pModel->index(i, 2), 0);
        m_pModel->setData(m_pModel->index(i, 3), 0);
    }
}

void TableNGSort::UpdateTable(StationInfo* info)
{
	QList<QPair<QString, int>> list;
	for (auto it = info->ngInfo.constBegin(); it != info->ngInfo.constEnd(); ++it) {
		list.append(qMakePair(it.key(), it.value()));
	}
	std::sort(list.begin(), list.end(), [](const QPair<QString, int>& a, const QPair<QString, int>& b) {
		// return a.second < b.second; // 升序 (从小到大)
		return a.second > b.second; // 降序 (从大到小)
		});

    for (int i = 0; i < m_pModel->rowCount(); i++) {
        if (i >= list.size()) {
            m_pModel->setData(m_pModel->index(i, 1), "");
            m_pModel->setData(m_pModel->index(i, 2), "");
            m_pModel->setData(m_pModel->index(i, 3), "");
        }
        else {
            double radio = 0;
            if (info->numTotal != 0)
                radio = list.at(i).second * 100.0 / info->numTotal;
            m_pModel->setData(m_pModel->index(i, 1), list.at(i).first);
            m_pModel->setData(m_pModel->index(i, 2), list.at(i).second);
            m_pModel->setData(m_pModel->index(i, 3), QString::asprintf("%.2f%", radio));
        }
    }
}
