#ifndef WidgetTrayState_H
#define WidgetTrayState_H

#include <QWidget>
#include <QTimer>
#include <QLabel>
#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include "ParamDef.h"


namespace Ui {
class WidgetTrayState;
}

class QCustomItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    QCustomItemDelegate(QObject* parent = nullptr);

    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
};


class WidgetTrayState : public QWidget
{
    Q_OBJECT

public:
    explicit WidgetTrayState(QWidget *parent = nullptr);
    ~WidgetTrayState();

public slots:
    int event_TrayUpdate(int flag, bool exits);
    int event_TrayHoleUpdate(TrayFunc func, bool blankFlag, bool ok, int number, QString text);
    //加载托盘码(预留)
    int event_ShowTayBarcode(TrayFunc func, QString barcode);
    int event_BoxTrayStatus(TrayFunc func, int layer, QString info);

private:
    void refreshStatusIcon(QLabel* iconLab, bool status);

    void Init();
    //表格大小
    void TabelInit();

private:
    Ui::WidgetTrayState *ui;
};

#endif // WidgetTrayState_H
