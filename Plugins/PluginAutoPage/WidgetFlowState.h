#ifndef WidgetFlowState_H
#define WidgetFlowState_H

#include <QWidget>
#include <QMap>
#include <QTableWidget>
#include <QLabel>
#include <QTimer>
#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QPixmap>
#include "ParamDef.h"
#include "../../Common/PipeLineDef.h"
namespace Ui {
class WidgetFlowState;
}

class CustomItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit CustomItemDelegate(QObject* parent = nullptr);
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
};

// 5个流线托盘映射绑定结构体
struct FlowTrayBind
{
    QTableWidget* table;
    QLabel* label;
    int rowParam;
    int colParam;
    TrayFunc func;
};

class WidgetFlowState : public QWidget
{
    Q_OBJECT
public:
    explicit WidgetFlowState(QWidget *parent = nullptr);
    ~WidgetFlowState();

public slots:
    // 下游Server状态更新
    int event_PipeLineServerStatusNotify(const DownStreamServerStatus& status);
    // 上游Client状态更新
    int event_PipeLineClientStatusNotify(const UpStreamClientStatus& status);

private slots:
    void slotRefreshTrayUI(); // 定时器定时刷新界面托盘状态

private:
    void Init();
    void TabelInit();
    void  InitOnlineType();
    // 通用创建表格标题Item
    QTableWidgetItem* CreateStatusTableItem(const QString& text, bool isTitle);
    void ConstructLinkStatusTable(QTableWidget* table,
                                  const QString& localGroupName, const QStringList& localTitles,
                                  const QString& peerGroupName, const QStringList& peerTitles,
                                  const QString& transGroupName, const QStringList& transTitles);

    void RefreshTableTitleText(QTableWidget* table,
                               const QString& localGroup, const QStringList& localTitles,
                               const QString& peerGroup, const QStringList& peerTitles,
                               const QString& transGroup, const QStringList& transTitles);


    void refreshSingleTrayLight(TrayFunc func, bool hasTray);
    void refreshSingleTrayText(TrayFunc func, const TrayInfo& trayInfo);

    // 下游联机表格初始化
    void InitDownstreamStatusTable();
    // 上游联机表格初始化
    void InitUpstreamStatusTable();
    // 刷新托盘单元格条码文字
    void refreshTrayCellText(QTableWidget* table, const TrayInfo& info);
    // 单灯刷新工具函数（指定表格）
    void UpdateStatusLight(QTableWidget* table, int row, int col, bool lightOn);

private:
    Ui::WidgetFlowState *ui;
    QMap<TrayFunc, FlowTrayBind> m_flowTrayMap;
    QTimer* m_uiRefreshTimer;
    // 联机布局
    const  bool LINK_DUAL_TABLE = false;
    const  bool LINK_IS_SERVER = true;
};

#endif // WidgetFlowState_H
