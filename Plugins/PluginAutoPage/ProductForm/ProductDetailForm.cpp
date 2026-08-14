#include "ProductDetailForm.h"
#include "ui_ProductDetailForm.h"
#include <QStyledItemDelegate>
#include <QPainter>

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
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
    {
        QStyleOptionViewItem opt(option);
        opt.rect.adjust(2, 2, -2, -2);
        QColor color = index.data(Qt::BackgroundColorRole).value<QColor>();
        if(color.isValid())
           painter->fillRect(opt.rect, color);
        QStyledItemDelegate::paint(painter, opt, index);
    }

};

ProductDetailForm::ProductDetailForm(int station,QWidget *parent) :
    QWidget(parent),
    m_station(station),
    ui(new Ui::ProductDetailForm)
{
    ui->setupUi(this);
    ui->groupBox->setTitle(QString(u8"治具%1-PCBA码").arg(station+1));
    InitTable();
}

ProductDetailForm::~ProductDetailForm()
{
    delete ui;
}

void ProductDetailForm::InitData(QString barcode)
{
    m_pModel->setRowCount(0);
}

void ProductDetailForm::AddData(ItemDetail &info)
{
    int rows=m_pModel->rowCount();
    m_pModel->insertRow(rows);
    m_pModel->setData(m_pModel->index(rows, 0), rows+1);
    m_pModel->setData(m_pModel->index(rows, 1), info.name+QString::number(rows+1));
    m_pModel->setData(m_pModel->index(rows, 2), QString::asprintf("%.3f", info.maxValue));
    m_pModel->setData(m_pModel->index(rows, 3), QString::asprintf("%.3f", info.minValue));
    m_pModel->setData(m_pModel->index(rows, 4), info.sUnit);
    m_pModel->setData(m_pModel->index(rows, 5), QString::asprintf("%.3f", info.testValue));
    m_pModel->setData(m_pModel->index(rows, 6), info.result?"Pass":"NG");
    QColor color=info.result?Qt::green:Qt::red;
    m_pModel->setData(m_pModel->index(rows, 6),color ,Qt::BackgroundColorRole);
}

void ProductDetailForm::InitTable()
{
    ui->tableView->verticalHeader()->setVisible(false);
    ui->tableView->horizontalHeader()->setVisible(false);
    ui->tableView->horizontalHeader()->setVisible(true);
    ui->tableView->verticalHeader()->setDefaultSectionSize(40);
    ui->tableView->setSelectionMode(QAbstractItemView::NoSelection);
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);  //自适应列宽
    ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);	//设置不可编辑
    QStringList headerList;
    headerList<< u8"序号" << u8"测试项"<<u8"最大值"<<u8"最小值"<<u8"单位"<<u8"测试值"<<u8"结果";
    m_pModel = new QStandardItemModel(this);
    m_pModel->setHorizontalHeaderLabels(headerList);
    ui->tableView->setItemDelegate(new CenterDelegate(this));
    ui->tableView->setModel(m_pModel);
}
