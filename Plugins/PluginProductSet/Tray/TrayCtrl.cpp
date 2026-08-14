#include "TrayCtrl.h"
#include "ui_TrayCtrl.h"
#include <QDebug>
#include <QStyleOptionViewItem>
#include <QStyle>
#include <QMouseEvent>
#include <QPainter>

QCustomItemDelegate::QCustomItemDelegate(QObject* parent)
	:QStyledItemDelegate(parent)
{

}

void QCustomItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	QStyleOptionViewItem opt(option);
	opt.rect.adjust(2, 2, -2, -2);
    QColor color = index.data(Qt::BackgroundColorRole).value<QColor>();
    painter->fillRect(opt.rect, color);
	QStyledItemDelegate::paint(painter, opt, index); 
}


TrayCtrl::TrayCtrl(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TrayCtrl)
{
    ui->setupUi(this);
    ui->tableViewTray->installEventFilter(this);
    //拦截鼠标事件必须使用viewport
    ui->tableViewTray->viewport()->installEventFilter(this);
}

TrayCtrl::~TrayCtrl()
{
    delete ui;
}

void TrayCtrl::setCtrlIndex(int index)
{
    m_nIndex = index;
}

void TrayCtrl::setTrayName(const QString &name,bool suffix)
{
    if (suffix) {
        ui->groupBoxTray->setTitle(name + u8"托盘");
        ui->groupBoxSilos->setTitle(name + u8"料仓");
    }
    else
        ui->groupBoxTray->setTitle(name);
}

void TrayCtrl::initTrayModel(int rows, int cols, QList<int> unUsedList)
{
    if (!m_pModelTray)
    {
        m_pModelTray = new QStandardItemModel(rows, cols, this);
        auto delegate = new QCustomItemDelegate(ui->tableViewTray);
        ui->tableViewTray->setItemDelegate(delegate);
        ui->tableViewTray->setModel(m_pModelTray);
        ui->tableViewTray->setShowGrid(false);
        ui->tableViewTray->horizontalHeader()->hide();
        ui->tableViewTray->verticalHeader()->hide();
        ui->tableViewTray->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        ui->tableViewTray->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        ui->tableViewTray->setSelectionMode(QAbstractItemView::SingleSelection);
		connect(ui->tableViewTray, &QTableView::clicked, this, [=](const QModelIndex& index) {
			qDebug() << "emit sigTraySelected(" << m_nIndex << m_nLayerIndex << index.row() << index.column() << ")";
			emit sigTraySelected(m_nIndex, m_nLayerIndex, index.row(), index.column());
			});
  //      ui->tableViewTray->setContextMenuPolicy(Qt::CustomContextMenu);
		//connect(ui->tableViewTray, &QTableView::customContextMenuRequested, this, [=](const QPoint&pt) {
  //          QModelIndex index = ui->tableViewTray->indexAt(pt);
  //          if (!index.isValid())return;
  //          //qDebug() << "emit sigTraySelected(" << m_nIndex << m_nLayerIndex << index.row() << index.column() << ")";
		//	emit sigTraySelected(m_nIndex, m_nLayerIndex, index.row(), index.column());
		//	});
    }
    else {
        m_pModelTray->setRowCount(rows);
        m_pModelTray->setColumnCount(cols);
    }
    initTrayCell(Qt::gray,unUsedList);
}

void TrayCtrl::GetTrayRowCol(int& rows, int& cols)
{
    rows = m_pModelTray->rowCount();
    cols = m_pModelTray->columnCount();
}

bool TrayCtrl::setTrayCellData(int row, int col, const QVariant &value, int role)
{
    if (m_pModelTray)
    {
        auto index = m_pModelTray->index(row, col);
        if (!index.isValid())
        {
            return false;
        }
        m_pModelTray->setData(index, value, role);
        return true;
    }
    return false;
}

QVariant TrayCtrl::getTrayCellData(int row, int col, int role)
{
    if (m_pModelTray)
    {
        auto index = m_pModelTray->index(row, col);
        if (!index.isValid())
        {
            return QVariant();
        }
        return m_pModelTray->data(index, role);
    }
    return QVariant();
}

void TrayCtrl::setTrayCellDefaultColor(const QColor &color)
{
    if (m_pModelTray)
    {
        int rows = m_pModelTray->rowCount();
        int cols = m_pModelTray->columnCount();
        for (int i = 0; i < rows; ++i)
        {
            for (int j = 0; j < cols; ++j)
            {
                auto index = m_pModelTray->index(i,j);
                m_pModelTray->setData(index, QVariant(color), Qt::BackgroundColorRole);
                //m_pModelTray->setData(index, QVariant(color), Qt::BackgroundColorRole);
            }
        }
    }
}

void TrayCtrl::RenameCellName()
{
    if (!m_pModelTray->rowCount())return;
    if (!m_pModelTray->columnCount())return;
    m_pModelTray->item(0, 0)->setText(QString("A"));
    m_pModelTray->item(0, 0)->setBackground(QBrush(QColor(255, 0, 0)));
    if(m_pModelTray->columnCount() - 1 != 0)
    {
        m_pModelTray->item(0, m_pModelTray->columnCount() - 1)->setText(QString("B"));
        m_pModelTray->item(0, m_pModelTray->columnCount() - 1)->setBackground(QBrush(QColor(255, 0, 0)));
    }
    if(m_pModelTray->rowCount() - 1 != 0)
    {
        m_pModelTray->item(m_pModelTray->rowCount() - 1, 0)->setText(QString("C"));
        m_pModelTray->item(m_pModelTray->rowCount() - 1, 0)->setBackground(QBrush(QColor(255, 0, 0)));
    }
}

void TrayCtrl::setSilosName(const QString &name)
{
    ui->groupBoxSilos->setTitle(name);
}

void TrayCtrl::initSilosModel(int rows)
{
    if (!m_pModelSilos)
    {
        m_pModelSilos = new QStandardItemModel(rows, 1, this);
        auto delegate = new QCustomItemDelegate(ui->tableViewSilos);
        ui->tableViewSilos->setItemDelegate(delegate);
        ui->tableViewSilos->setModel(m_pModelSilos);
        ui->tableViewSilos->setShowGrid(false);
        ui->tableViewSilos->horizontalHeader()->hide();
        ui->tableViewSilos->verticalHeader()->hide();
        ui->tableViewSilos->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        ui->tableViewSilos->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        ui->tableViewSilos->setSelectionMode(QAbstractItemView::SingleSelection);

        int index = 1;
        for (int i = 0; i < rows; ++i)
        {
            m_pModelSilos->setItem(i,0, new QStandardItem(QString::number(index++)));
            m_pModelSilos->item(i,0)->setData(Qt::AlignCenter, Qt::TextAlignmentRole);
            m_pModelSilos->item(i,0)->setBackground(Qt::gray);
            m_pModelSilos->item(i,0)->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        }
        connect(ui->tableViewSilos, &QTableView::clicked, this, [=](const QModelIndex &index){
            //qDebug() << "emit sigSilosSelected(" << m_nIndex << index.row() <<  ")";
            emit sigSilosSelected(m_nIndex, index.row());
        });
    }
    initSilosLayer();
}

bool TrayCtrl::setSilosCellData(int row, const QVariant &value, int role)
{
    if (m_pModelSilos)
    {
        auto index = m_pModelSilos->index(row, 0);
        if (!index.isValid())
        {
            return false;
        }
        m_pModelSilos->setData(index, value, role);
        return true;
    }
    return false;
}

QVariant TrayCtrl::getSilosCellData(int row, int role)
{
    if (m_pModelSilos)
    {
        auto index = m_pModelSilos->index(row, 0);
        if (!index.isValid())
        {
            return QVariant();
        }
        return m_pModelSilos->data(index, role);
    }
    return QVariant();
}

void TrayCtrl::setSilosCellDefaultColor(const QColor &color)
{
    if (m_pModelSilos)
    {
        int rows = m_pModelSilos->rowCount();
        int cols = m_pModelSilos->columnCount();
        for (int i = 0; i < rows; ++i)
        {
            for (int j = 0; j < cols; ++j)
            {
                auto index = m_pModelSilos->index(i,j);
                m_pModelSilos->setData(index, QVariant(color), Qt::BackgroundColorRole);
            }
        }
    }
}

void TrayCtrl::initSilosLayer()
{
    int rows = m_pModelSilos->rowCount();
	for (int i = 0; i < rows; ++i){
		QString strText = QString::fromLocal8Bit("有盘待使用 - %1").arg(i + 1);
		setSilosCellData(i, QVariant(strText), Qt::EditRole);
		setSilosCellData(i, QVariant(QColor(Qt::green)), Qt::BackgroundColorRole);
        setSilosCellData(i, true, Qt::UserRole + 1);
	}
}

void TrayCtrl::initTrayCell(QColor color, QList<int> unUsedList)
{
    int cols = m_pModelTray->columnCount();
    int rows = m_pModelTray->rowCount();
    for (int i = 0; i < rows; ++i){
        for (int j = 0; j < cols; ++j){

            int index = i * cols + j + 1;
            QString text = QString::number(index);
            //查找不使用穴位
            bool bUnUsed = false;
            for(int k = 0;k<unUsedList.size();++k )
            {
                if(index == unUsedList.at(k))
                {
                    bUnUsed = true;
                    break;
                }
            }
            if(bUnUsed)
            {
                text = "--";
                color = Qt::gray;
            }
            else
            {
                color = Qt::green;
            }
            m_pModelTray->setItem(i, j, new QStandardItem(text));
			m_pModelTray->item(i, j)->setData(Qt::AlignCenter, Qt::TextAlignmentRole);
			m_pModelTray->item(i, j)->setBackground(color);
			m_pModelTray->item(i, j)->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
            if (m_flagTrayEdit)
                m_pModelTray->item(i, j)->setFlags(m_pModelTray->item(i, j)->flags() | Qt::ItemIsEditable);
		}
	}
}

void TrayCtrl::SetTrayAttribute(bool visible, bool bEdit)
{
    if (!visible)
        ui->groupBoxTray->setStyleSheet("QGroupBox {border: 0px solid transparent;}"
                                        "QGroupBox {border: 0;}"
                                        "QGroupBox:title{ max-width: 0; max-height: 0; }");
    ui->groupBoxSilos->setVisible(visible);
    m_flagTrayEdit = bEdit;
}

void TrayCtrl::SetSelectFlag(bool select)
{
    ui->tableViewSilos->setSelectionMode(select ? QAbstractItemView::SingleSelection : QAbstractItemView::NoSelection);
    ui->tableViewTray->setSelectionMode(select ? QAbstractItemView::SingleSelection : QAbstractItemView::NoSelection);
}

void TrayCtrl::ClearSelect()
{
    ui->tableViewTray->clearSelection();
    ui->tableViewSilos->clearSelection();
}

QPixmap TrayCtrl::GrabTrayImg()
{
    return ui->tableViewTray->grab();
}

void TrayCtrl::SetSilosVisible(bool visible)
{
    if (!visible)
    {
        if(ui->groupBoxTray->title().isEmpty())
        {
            ui->groupBoxTray->setStyleSheet("QGroupBox {border: 0px solid transparent;margin-top:0px;}"
                                            "QGroupBox {border: 0;}"
                                            "QGroupBox:title{ max-width: 0; max-height: 0; }");
        }
        else
        {
            ui->groupBoxTray->setStyleSheet("QGroupBox {border: 0px solid transparent;}"
                                            "QGroupBox {border: 0;}"
                                            "QGroupBox:title{ max-width: 0; max-height: 0; }");
        }
    }

    ui->groupBoxSilos->setVisible(visible);
}

