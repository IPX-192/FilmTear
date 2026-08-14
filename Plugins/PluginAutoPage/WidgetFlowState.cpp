#include "WidgetFlowState.h"
#include "ui_WidgetFlowState.h"
#include "ParamManager.h"
#include <QPainter>
#include <QPixmap>
#include <QtCore/QMetaType>
#include <QFont>
#include <QDebug>
#include "VisAppBus.h"

Q_DECLARE_METATYPE(TrayFunc)
Q_DECLARE_METATYPE(TrayInfo)
Q_DECLARE_METATYPE(DownStreamServerStatus)
Q_DECLARE_METATYPE(UpStreamClientStatus)

Q_DECLARE_METATYPE(QVector<TrayFunc>)
Q_DECLARE_METATYPE(QVector<bool>)
Q_DECLARE_METATYPE(QVector<TrayInfo>)

// 全局灯尺寸常量
const int LIGHT_SIZE = 50;
// 全局颜色规范
const  QColor COLOR_TITLE_BG = QColor(220, 220, 220); // 标题行浅灰背景
const  QColor COLOR_CELL_BG = QColor(255, 255, 255);  // 单元格白色背景
const  QColor COLOR_TRAY_GREEN = QColor(0, 255, 0);   // 托盘OK绿色
const  QColor COLOR_TRAY_RED = QColor(255, 0, 0);     // 托盘NG红色
const  QColor COLOR_TRAY_GRAY = QColor(125, 125, 125); // 托盘空盘灰色

const  QStringList localTitles_Down = {QStringLiteral("本站出口有盘"), QStringLiteral("本站回流要料"), QStringLiteral("回流托盘到达")};
const  QStringList localTitles_Up = {QStringLiteral("输送线本站要料"), QStringLiteral("上游托盘到达"), QStringLiteral("本站回流有盘")};
const  QStringList transTitles = {QStringLiteral("输送线传输托盘"), QStringLiteral("回流线传输托盘")};


CustomItemDelegate::CustomItemDelegate(QObject* parent)
    :QStyledItemDelegate(parent)
{

}

void CustomItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QStyleOptionViewItem opt(option);
    opt.rect.adjust(2, 2, -2, -2);
    bool isStatusLight = index.data(Qt::UserRole).toBool();

    if (isStatusLight)
    {
        painter->fillRect(opt.rect, COLOR_CELL_BG);
        bool lightOn = index.data(Qt::DisplayRole).toBool();
        QString resPath = lightOn ? ":/icon/greenlight.png" : ":/icon/greylight.png";
        QPixmap pix(resPath);
        QRect drawRect;
        drawRect.setSize(QSize(LIGHT_SIZE, LIGHT_SIZE));
        drawRect.moveCenter(opt.rect.center());
        painter->drawPixmap(drawRect, pix.scaled(LIGHT_SIZE, LIGHT_SIZE, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        return;
    }
    else
    {
        QColor color = index.data(Qt::BackgroundColorRole).value<QColor>();
        painter->fillRect(opt.rect, color);
    }
    QStyledItemDelegate::paint(painter, opt, index);
}

WidgetFlowState::WidgetFlowState(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WidgetFlowState)
{
    ui->setupUi(this);
    VisAppBus::subscibeEvent(this, "PipeLineServerStatusNotify");
    VisAppBus::subscibeEvent(this, "PipeLineClientStatusNotify");
    qRegisterMetaType<TrayFunc>("TrayFunc");
    qRegisterMetaType<QVector<TrayFunc>>();
    qRegisterMetaType<QVector<bool>>();
    qRegisterMetaType<QVector<TrayInfo>>();
    Init();
    InitDownstreamStatusTable();
    InitUpstreamStatusTable();
    InitOnlineType();

    m_uiRefreshTimer = new QTimer(this);
    m_uiRefreshTimer->setInterval(1000); // 1000ms=1秒
    connect(m_uiRefreshTimer, &QTimer::timeout, this, &WidgetFlowState::slotRefreshTrayUI);
    m_uiRefreshTimer->start();
}

WidgetFlowState::~WidgetFlowState()
{
    if(m_uiRefreshTimer)
    {
        delete  m_uiRefreshTimer;
        m_uiRefreshTimer = nullptr;
    }
    delete ui;
}

void WidgetFlowState::Init()
{
    m_flowTrayMap[EmptyBuf] = {
        ui->tableWidgetEmptyBuf,
        ui->labelEmptyBuf,
        1,
        1,
        EmptyBuf
    };
    m_flowTrayMap[FeedHolder] = {
        ui->tableWidgetFeedHolderMid,
        ui->labelFeedHolderMid,
        1,
        1,
        FeedHolder
    };
    m_flowTrayMap[FeedPCB] = {
        ui->tableWidgetFeedPCBMid,
        ui->labelFeedPCBMid,
        1,
        1,
        FeedPCB
    };
    m_flowTrayMap[PCBClean] = {
        ui->tableWidgetPCBClean,
        ui->labelPCBClean,
        1,
        1,
        PCBClean
    };
    m_flowTrayMap[FeedTurntable] = {
        ui->tableWidgetFeedTurntable,
        ui->labelFeedTurntable,
        1,
        1,
        FeedTurntable
    };

    TabelInit();
}

void WidgetFlowState::TabelInit()
{
    for (auto& bind : m_flowTrayMap)
    {
        QTableWidget* w = bind.table;
        w->clear();
        w->setItemDelegate(new CustomItemDelegate(this));
        w->setRowCount(1);
        w->setColumnCount(1);

        w->verticalHeader()->hide();
        w->horizontalHeader()->hide();
        w->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        w->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        w->setWordWrap(true);

        QTableWidgetItem *item = new QTableWidgetItem();
        w->setItem(0, 0, item);
        item->setFont(QFont("song", 11));
        item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        item->setFlags(Qt::NoItemFlags);
        item->setText(u8"无托盘");
        item->setBackgroundColor(COLOR_TRAY_GRAY);
        item->setTextColor(Qt::black);
    }
}

void WidgetFlowState::InitOnlineType()
{

    if (LINK_DUAL_TABLE)
    {

        ui->tableWidgetDownFlow->setVisible(true);
        ui->tableWidgetUpFlow->setVisible(true);
        ui->labelOnlineLeft->setVisible(true);
        ui->labelOnlineRight->setVisible(true);

        ui->labelOnlineLeft->setText(QStringLiteral("和下游联机"));
        ui->labelOnlineRight->setText(QStringLiteral("和上游联机"));

        QStringList upPeerTitles;
        upPeerTitles << QStringLiteral("上游有料") << QStringLiteral("上游回流要料") << QStringLiteral("回流托盘到上游");
        RefreshTableTitleText(ui->tableWidgetUpFlow,
                              QStringLiteral("本机信号"), localTitles_Up,
                              QStringLiteral("上游信号"), upPeerTitles,
                              QStringLiteral("传输状态"), transTitles);


        QStringList downPeerTitles;
        downPeerTitles << QStringLiteral("下游要盘") << QStringLiteral("下游回流有盘") << QStringLiteral("托盘到位下游");
        RefreshTableTitleText(ui->tableWidgetDownFlow,
                              QStringLiteral("本机信号"), localTitles_Down,
                              QStringLiteral("下游信号"), downPeerTitles,
                              QStringLiteral("传输状态"), transTitles);
        return ;
    }
    else
    {

        ui->tableWidgetDownFlow->setVisible(true);
        ui->tableWidgetUpFlow->setVisible(false);
        ui->labelOnlineRight->setVisible(false);
        ui->labelOnlineLeft->setVisible(true);
    }

    // 仅单表格模式才根据serverStatus切换左侧文字
    QString peerGroup;
    QStringList peerTitles;
    QStringList useLocalTitles;
    if (LINK_IS_SERVER)
    {
        ui->labelOnlineLeft->setText(QStringLiteral("和下游联机"));
        peerGroup = QStringLiteral("下游信号");
        peerTitles << QStringLiteral("下游要盘") << QStringLiteral("下游回流有盘") << QStringLiteral("托盘到位下游");
        useLocalTitles = localTitles_Down;
    }
    else
    {
        ui->labelOnlineLeft->setText(QStringLiteral("和上游联机"));
        peerGroup = QStringLiteral("上游信号");
        peerTitles << QStringLiteral("上游有料") << QStringLiteral("上游回流要料") << QStringLiteral("回流托盘到上游");
        useLocalTitles = localTitles_Up;
    }
    RefreshTableTitleText(ui->tableWidgetDownFlow,
                          QStringLiteral("本机信号"), useLocalTitles,
                          peerGroup, peerTitles,
                          QStringLiteral("传输状态"), transTitles);
}

QTableWidgetItem* WidgetFlowState::CreateStatusTableItem(const QString& text, bool isTitle)
{
    QTableWidgetItem* item = new QTableWidgetItem(text);
    item->setFlags(Qt::NoItemFlags);
    item->setTextAlignment(Qt::AlignCenter);
    QFont font("Microsoft YaHei", 12);
    if (isTitle)
    {
        item->setBackgroundColor(COLOR_TITLE_BG);
    }
    item->setFont(font);
    item->setTextColor(Qt::black);

    if (!isTitle)
    {
        item->setBackgroundColor(COLOR_CELL_BG);
    }
    return item;
}

void WidgetFlowState::ConstructLinkStatusTable(QTableWidget* table,
                                               const QString& localGroupName, const QStringList& localTitles,
                                               const QString& peerGroupName, const QStringList& peerTitles,
                                               const QString& transGroupName, const QStringList& transTitles)
{
    table->clear();
    table->setItemDelegate(new CustomItemDelegate(this));
    table->setRowCount(6);
    table->setColumnCount(4);

    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionMode(QAbstractItemView::NoSelection);
    table->verticalHeader()->hide();
    table->horizontalHeader()->hide();
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    table->setRowHeight(0, 30);
    table->setRowHeight(1, 70);
    table->setRowHeight(2, 30);
    table->setRowHeight(3, 70);
    table->setRowHeight(4, 30);
    table->setRowHeight(5, 70);

    table->setSpan(0, 0, 2, 1);
    table->setSpan(2, 0, 2, 1);
    table->setSpan(4, 0, 2, 1);

    // 本机信号行
    table->setItem(0, 0, CreateStatusTableItem(localGroupName, true));
    for (int col = 1; col <= 3; col++)
    {
        QString title = (col - 1 < localTitles.size()) ? localTitles[col - 1] : "";
        table->setItem(0, col, CreateStatusTableItem(title, true));
    }
    for (int col = 1; col <= 3; col++)
    {
        QTableWidgetItem* light = new QTableWidgetItem("");
        light->setData(Qt::UserRole, true);
        light->setData(Qt::DisplayRole, false);
        light->setFlags(Qt::NoItemFlags);
        light->setBackgroundColor(COLOR_CELL_BG);
        table->setItem(1, col, light);
    }

    // 上下游对等信号行
    table->setItem(2, 0, CreateStatusTableItem(peerGroupName, true));
    for (int col = 1; col <= 3; col++)
    {
        QString title = (col - 1 < peerTitles.size()) ? peerTitles[col - 1] : "";
        table->setItem(2, col, CreateStatusTableItem(title, true));
    }
    for (int col = 1; col <= 3; col++)
    {
        QTableWidgetItem* light = new QTableWidgetItem("");
        light->setData(Qt::UserRole, true);
        light->setData(Qt::DisplayRole, false);
        light->setFlags(Qt::NoItemFlags);
        light->setBackgroundColor(COLOR_CELL_BG);
        table->setItem(3, col, light);
    }

    // 传输状态行
    table->setItem(4, 0, CreateStatusTableItem(transGroupName, true));
    for (int col = 1; col <= 3; col++)
    {
        QString title = (col - 1 < transTitles.size()) ? transTitles[col - 1] : "";
        table->setItem(4, col, CreateStatusTableItem(title, true));
    }

    for (int col = 1; col <= 2; col++)
    {
        QTableWidgetItem* light = new QTableWidgetItem("");
        light->setData(Qt::UserRole, true);
        light->setData(Qt::DisplayRole, false);
        light->setFlags(Qt::NoItemFlags);
        light->setBackgroundColor(COLOR_CELL_BG);
        table->setItem(5, col, light);
    }
    //最后一格单独处理
    QTableWidgetItem* empty = new QTableWidgetItem("");
    empty->setBackgroundColor(COLOR_CELL_BG);
    table->setItem(5, 3, empty);
}

void WidgetFlowState::RefreshTableTitleText(QTableWidget* table,
                                            const QString& localGroup, const QStringList& localTitles,
                                            const QString& peerGroup, const QStringList& peerTitles,
                                            const QString& transGroup, const QStringList& transTitles)
{

    table->item(0, 0)->setText(localGroup);
    table->item(2, 0)->setText(peerGroup);
    table->item(4, 0)->setText(transGroup);
    for (int col = 1; col <= 3; col++)
    {
        QString title = (col - 1 < localTitles.size()) ? localTitles[col - 1] : "";
        table->item(0, col)->setText(title);
    }
    for (int col = 1; col <= 3; col++)
    {
        QString title = (col - 1 < peerTitles.size()) ? peerTitles[col - 1] : "";
        table->item(2, col)->setText(title);
    }
    for (int col = 1; col <= 3; col++)
    {
        QString title = (col - 1 < transTitles.size()) ? transTitles[col - 1] : "";
        table->item(4, col)->setText(title);
    }
}

void WidgetFlowState::refreshSingleTrayLight(TrayFunc func, bool hasTray)
{
    if (!m_flowTrayMap.contains(func))
        return;

    auto bind = m_flowTrayMap[func];
    QTableWidget *widget = bind.table;
    QLabel *label = bind.label;
    QTableWidgetItem *cellItem = widget->item(0, 0);
    if (!cellItem) return;

    if (!hasTray)
    {
        cellItem->setText(u8"无托盘");
        cellItem->setBackgroundColor(COLOR_TRAY_GRAY);
    }
    else
    {
        cellItem->setBackgroundColor(COLOR_TRAY_GREEN);
    }

    label->setStyleSheet(hasTray ? "background-color: rgb(0,255,0);" : "background-color: rgb(219,219,219);");
}

void WidgetFlowState::refreshSingleTrayText(TrayFunc func, const TrayInfo& trayInfo)
{
    if (!m_flowTrayMap.contains(func))
        return;
    auto bind = m_flowTrayMap[func];
    refreshTrayCellText(bind.table, trayInfo);
}

void WidgetFlowState::InitDownstreamStatusTable()
{
    QTableWidget* table = ui->tableWidgetDownFlow;
    QStringList titleDown  = {QStringLiteral("下游要盘"), QStringLiteral("下游回流有盘"), QStringLiteral("托盘到位下游")};
    ConstructLinkStatusTable(table,
                             QStringLiteral("本机信号"), localTitles_Down,
                             QStringLiteral("下游信号"), titleDown,
                             QStringLiteral("传输状态"), transTitles);
}

void WidgetFlowState::InitUpstreamStatusTable()
{
    QTableWidget* table = ui->tableWidgetUpFlow;
    QStringList titleUp    = {QStringLiteral("上游有料"), QStringLiteral("上游回流要料"), QStringLiteral("回流托盘到上游")};
    ConstructLinkStatusTable(table,
                             QStringLiteral("本机信号"), localTitles_Up,
                             QStringLiteral("上游信号"), titleUp,
                             QStringLiteral("传输状态"), transTitles);
}

void WidgetFlowState::refreshTrayCellText(QTableWidget* table, const TrayInfo& info)
{
    if (!table) return;
    QTableWidgetItem* cellItem = table->item(0, 0);
    if (!cellItem) return;

    QString text;
    if (info.empty)
    {
        text = QString(u8"托盘码:%1").arg(info.trayCode);
        cellItem->setBackgroundColor(COLOR_TRAY_GREEN);
    }
    else
    {
        text += QString(u8"托盘码:%1\n").arg(info.trayCode);
        if (!info.holderBarCode.isEmpty())
            text += QString(u8"壳体:%1\n").arg(info.holderBarCode);
        if (!info.pcbBarCode.isEmpty())
            text += QString(u8"PCB:%1\n").arg(info.pcbBarCode);
        if (!info.lensBarCode.isEmpty())
            text += QString(u8"镜头:%1\n").arg(info.lensBarCode);

        if (!info.result && !info.errInfo.isEmpty())
        {
            text += QString(u8"【异常】%1").arg(info.errInfo);
        }

        if (info.result)
            cellItem->setBackgroundColor(COLOR_TRAY_GREEN);
        else
            cellItem->setBackgroundColor(COLOR_TRAY_RED);
    }
    cellItem->setText(text);
}

void WidgetFlowState::UpdateStatusLight(QTableWidget* table, int row, int col, bool lightOn)
{
    if (!table) return;
    QTableWidgetItem* item = table->item(row, col);
    if (!item) return;
    item->setData(Qt::DisplayRole, lightOn);
}

int WidgetFlowState::event_PipeLineServerStatusNotify(const DownStreamServerStatus &status)
{
    //单模式的客户端数据直接返回
    if(!LINK_IS_SERVER && !LINK_DUAL_TABLE)
    {
        return  0;
    }
    // 无论单/双模式，服务端数据都渲染左侧表格
    QTableWidget* targetTable = ui->tableWidgetDownFlow;

    UpdateStatusLight(targetTable, 1, 1, status.m_existTray);
    UpdateStatusLight(targetTable, 1, 2, status.m_requestTrayBackFlow);
    UpdateStatusLight(targetTable, 1, 3, status.m_trayToBackFlow);

    UpdateStatusLight(targetTable, 3, 1, status.m_nextRequest);
    UpdateStatusLight(targetTable, 3, 2, status.m_nextBackFlowExist);
    UpdateStatusLight(targetTable, 3, 3, status.m_trayInNextPos);

    UpdateStatusLight(targetTable, 5, 1, status.m_transTray);
    UpdateStatusLight(targetTable, 5, 2, status.m_transTrayBackFlow);
    return 0;
}

int WidgetFlowState::event_PipeLineClientStatusNotify(const UpStreamClientStatus &status)
{
    //单模式的服务端数据直接返回
    if(LINK_IS_SERVER && !LINK_DUAL_TABLE)
    {
        return  0;
    }
    QTableWidget* targetTable;
    if (LINK_DUAL_TABLE)
    {
        // 双表格：上游单独右侧
        targetTable = ui->tableWidgetUpFlow;
    }
    else
    {
        // 单表格：上游也放左侧
        targetTable = ui->tableWidgetDownFlow;
    }

    UpdateStatusLight(targetTable, 1, 1, status.m_trayRequest);
    UpdateStatusLight(targetTable, 1, 2, status.m_trayInCurPos);
    UpdateStatusLight(targetTable, 1, 3, status.m_existTrayBackFlow);

    UpdateStatusLight(targetTable, 3, 1, status.m_preExistTray);
    UpdateStatusLight(targetTable, 3, 2, status.m_preRequestBackFlow);
    UpdateStatusLight(targetTable, 3, 3, status.m_trayInNextPosBackFlow);

    UpdateStatusLight(targetTable, 5, 1, status.m_transTray);
    UpdateStatusLight(targetTable, 5, 2, status.m_transTrayBackFlow);
    return 0;
}

void WidgetFlowState::slotRefreshTrayUI()
{
    QVector<TrayFunc> vecFunc;
    QVector<bool> vecHasTray;
    QVector<TrayInfo> vecTrayData;

    int busRet = VisAppBus::sendEventDirect("RefreshAllTrayMap", vecFunc,vecHasTray,vecTrayData);

    if (busRet != 0)
    {
        return;
    }

    for (int i = 0; i < vecFunc.size(); i++)
    {
        TrayFunc type = vecFunc[i];
        bool hasTray = vecHasTray[i];
        TrayInfo trayData = vecTrayData[i];
        refreshSingleTrayLight(type, hasTray);
        if (hasTray)
        {
            refreshSingleTrayText(type, trayData);
        }
    }
}
