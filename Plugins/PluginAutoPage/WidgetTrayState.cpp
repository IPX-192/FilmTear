#include "WidgetTrayState.h"
#include "ui_WidgetTrayState.h"
#include "ParamManager.h"
#include <QPainter>
#include <QFont>
#include <QtCore/QMetaType>
#include "VisAppBus.h"

Q_DECLARE_METATYPE(TrayFunc)

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

WidgetTrayState::WidgetTrayState(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WidgetTrayState)
{
    ui->setupUi(this);

    VisAppBus::subscibeEvent(this, "TrayUpdate");
    VisAppBus::subscibeEvent(this, "TrayHoleUpdate");
    VisAppBus::subscibeEvent(this, "BoxTrayStatus");

    Init();
    // 初始化全部表格为无盘灰色
    event_TrayUpdate(0, false);
}

WidgetTrayState::~WidgetTrayState()
{
    delete ui;
}

// 整盘状态刷新：flag=0初始化；1壳体取料盘；2PCB取料盘；3壳体夹层；4PCB夹层
int WidgetTrayState::event_TrayUpdate(int flag, bool exits)
{
    QTableWidget *widget = nullptr;
    QLabel *label = nullptr;
    int curColCnt = 1;

    if (flag == 0)
    {
        TabelInit();
        return 0;
    }
    else if (flag == 1)
    {
        widget = ui->tableWidgetFeedHolder;
        label = ui->labelFeedHolder;
        curColCnt = GlobalParam->recipeTray.trayColsHolder;
    }
    else if (flag == 2)
    {
        widget = ui->tableWidgetFeedPCB;
        label = ui->labelFeedPCB;
        curColCnt = GlobalParam->recipeTray.trayColsPCB;
    }
    else if (flag == 3)
    {
        widget = ui->tableWidgetHolderLayer;
        label = ui->labelHolderLayer;
        curColCnt = GlobalParam->recipeTray.holderLayerCols;
    }
    else if (flag == 4)
    {
        widget = ui->tableWidgetPCBLayer;
        label = ui->labelPCBLayer;
        curColCnt = GlobalParam->recipeTray.pcBLayerCols;
    }

    if (!widget || !label)
        return -1;

    // 批量刷新所有格子底色，初始显示数字编号
    for (int i = 0; i < widget->rowCount(); i++)
    {
        for (int j = 0; j < widget->columnCount(); j++)
        {
            QTableWidgetItem *item = widget->item(i, j);
            if(!item) continue;
            int num = i * curColCnt + j + 1;
            item->setText(QString::number(num));
            item->setBackgroundColor(exits ? QColor(0, 255, 0) : QColor(125, 125, 125));
        }
    }
    return 0;
}

int WidgetTrayState::event_BoxTrayStatus(TrayFunc func, int layer, QString info)
{
    QTableWidget* tableWidget = nullptr;
    int colCnt = 0;
    QTableWidget* trayTable = nullptr;
    int trayRows, trayCols;
    if (func == FeedHolder)
    {
        tableWidget = ui->tableWidgetHolderLayer;
        colCnt = GlobalParam->recipeTray.holderLayerCols;
        trayTable = ui->tableWidgetFeedHolder;
        trayRows = GlobalParam->recipeTray.trayRowsHolder;
        trayCols = GlobalParam->recipeTray.trayColsHolder;
    }
    else if (func == FeedPCB)
    {
        tableWidget = ui->tableWidgetPCBLayer;
        colCnt = GlobalParam->recipeTray.pcBLayerCols;
        trayTable = ui->tableWidgetFeedPCB;
        trayRows = GlobalParam->recipeTray.trayRowsPCB;
        trayCols = GlobalParam->recipeTray.trayColsPCB;
    }
    else
        return -1;

    if (!tableWidget || !trayTable)
        return -1;

    // 更新夹层单行
    int row = layer / colCnt;
    int col = layer % colCnt;
    QTableWidgetItem* item = tableWidget->item(row, col);
    if (!item)
        return -1;
    int showNum = layer + 1;
    if (info == u8"有盘")
    {
        item->setBackgroundColor(QColor(0, 255, 0));
        item->setText(QString::number(showNum));
        // 当前夹层layer有料，大盘全部格子恢复数字+绿色
        for (int i = 0; i < trayRows; i++)
        {
            for (int j = 0; j < trayCols; j++)
            {
                QTableWidgetItem* trayItem = trayTable->item(i, j);
                if (trayItem)
                {
                    int idx = i * trayCols + j + 1;
                    trayItem->setBackgroundColor(QColor(0, 255, 0));
                    trayItem->setText(QString::number(idx));
                }
            }
        }
    }
    else
    {
        //这里显示空盘和无盘
        item->setBackgroundColor(QColor(125, 125, 125));
        item->setText(info);
        // 当前夹层layer对应大盘全部格子置空
        for (int i = 0; i < trayRows; i++)
        {
            for (int j = 0; j < trayCols; j++)
            {
                QTableWidgetItem* trayItem = trayTable->item(i, j);
                if (trayItem)
                {
                    trayItem->setBackgroundColor(QColor(125, 125, 125));
                    trayItem->setText(u8"空");
                }
            }
        }
    }
    return 0;
}
int WidgetTrayState::event_TrayHoleUpdate(TrayFunc func, bool blankFlag, bool ok, int number, QString text)
{
    int row = 0, col = 0;
    QTableWidget* tableWidget = nullptr;
    int colCnt = 0;

    if (func == FeedHolder)
    {
        tableWidget = ui->tableWidgetFeedHolder;
        colCnt = GlobalParam->recipeTray.trayColsHolder;
    }
    else if (func == FeedPCB)
    {
        tableWidget = ui->tableWidgetFeedPCB;
        colCnt = GlobalParam->recipeTray.trayColsPCB;
    }
    else
        return -1;

    if (!tableWidget) return -1;
    row = number / colCnt;
    col = number % colCnt;
    QTableWidgetItem* item = tableWidget->item(row, col);
    if (!item) return -1;

    int holeNum = number + 1;
    if (!blankFlag)
    {
        item->setBackgroundColor(QColor(0, 255, 0));
        item->setText(QString::number(holeNum));
    }
    else
    {

        item->setBackgroundColor(QColor(125, 125, 125));
        item->setText(u8"空");
    }
    return 0;
}

int WidgetTrayState::event_ShowTayBarcode(TrayFunc func, QString barcode)
{
    if (func == FeedHolder)
    {
        ui->labelFeedHolder->setText(QString(u8"壳体取料盘(条码：%1)").arg(barcode));
    }
    else if (func == FeedPCB)
    {
        ui->labelFeedPCB->setText(QString(u8"PCB取料盘(条码：%1)").arg(barcode));
    }
    return 0;
}

void WidgetTrayState::Init()
{
    TabelInit();
}

void WidgetTrayState::TabelInit()
{
    // 1、壳体、PCB取料大盘初始化
    for (int index = 0; index < 2; index++)
    {
        QTableWidget *widget = nullptr;
        int rowCnt = 0, colCnt = 0;
        if (index == 0)
        {
            widget = ui->tableWidgetFeedHolder;
            rowCnt = GlobalParam->recipeTray.trayRowsHolder;
            colCnt = GlobalParam->recipeTray.trayColsHolder;
        }
        else
        {
            widget = ui->tableWidgetFeedPCB;
            rowCnt = GlobalParam->recipeTray.trayRowsPCB;
            colCnt = GlobalParam->recipeTray.trayColsPCB;
        }

        widget->clear();
        widget->setItemDelegate(new QCustomItemDelegate(this));
        widget->setRowCount(rowCnt);
        widget->setColumnCount(colCnt);
        widget->verticalHeader()->hide();
        widget->horizontalHeader()->hide();
        widget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        widget->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        widget->setWordWrap(true);

        for (int i = 0; i < widget->rowCount(); i++)
        {
            for (int j = 0; j < widget->columnCount(); j++)
            {
                QTableWidgetItem *item = new QTableWidgetItem();
                widget->setItem(i, j, item);
                item->setFont(QFont("song", 12));
                // 文字水平垂直居中
                item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
                item->setFlags(Qt::NoItemFlags);
                int num = i * colCnt + j + 1;
                item->setText(QString::number(num));
                // 初始化默认绿色
                item->setBackgroundColor(QColor(0, 255, 0));
                item->setTextColor(QColor(0, 0, 0));
            }
        }
    }

    // 2、壳体夹层
    QTableWidget* wHolderLayer = ui->tableWidgetHolderLayer;
    int hLayerRow = GlobalParam->recipeTray.holderLayerRows;
    int hLayerCol = GlobalParam->recipeTray.holderLayerCols;
    wHolderLayer->clear();
    wHolderLayer->setItemDelegate(new QCustomItemDelegate(this));
    wHolderLayer->setRowCount(hLayerRow);
    wHolderLayer->setColumnCount(hLayerCol);
    wHolderLayer->verticalHeader()->hide();
    wHolderLayer->horizontalHeader()->hide();
    wHolderLayer->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    wHolderLayer->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    wHolderLayer->setWordWrap(true);
    for (int i = 0; i < wHolderLayer->rowCount(); i++)
    {
        for (int j = 0; j < wHolderLayer->columnCount(); j++)
        {
            QTableWidgetItem *item = new QTableWidgetItem();
            wHolderLayer->setItem(i, j, item);
            item->setFont(QFont("song", 12));
            item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
            item->setFlags(Qt::NoItemFlags);
            item->setText(u8"有料");
            item->setBackgroundColor(QColor(0, 255, 0));
            item->setTextColor(QColor(0, 0, 0));
        }
    }

    // 3、PCB夹层
    QTableWidget* wPcblayer = ui->tableWidgetPCBLayer;
    int pLayerRow = GlobalParam->recipeTray.pcBLayerRows;
    int pLayerCol = GlobalParam->recipeTray.pcBLayerCols;
    wPcblayer->clear();
    wPcblayer->setItemDelegate(new QCustomItemDelegate(this));
    wPcblayer->setRowCount(pLayerRow);
    wPcblayer->setColumnCount(pLayerCol);
    wPcblayer->verticalHeader()->hide();
    wPcblayer->horizontalHeader()->hide();
    wPcblayer->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    wPcblayer->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    wPcblayer->setWordWrap(true);
    for (int i = 0; i < wPcblayer->rowCount(); i++)
    {
        for (int j = 0; j < wPcblayer->columnCount(); j++)
        {
            QTableWidgetItem *item = new QTableWidgetItem();
            wPcblayer->setItem(i, j, item);
            item->setFont(QFont("song", 12));
            item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
            item->setFlags(Qt::NoItemFlags);
            item->setText(u8"有料");
            item->setBackgroundColor(QColor(0, 255, 0));
            item->setTextColor(QColor(0, 0, 0));
        }
    }
}
