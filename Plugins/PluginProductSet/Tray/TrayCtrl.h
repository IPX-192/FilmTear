#ifndef TRAYCTRL_H
#define TRAYCTRL_H

#include <QWidget>
#include <QStandardItemModel>
#include <QStyledItemDelegate>


class QCustomItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    QCustomItemDelegate(QObject* parent = nullptr);

    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
};

namespace Ui {
class TrayCtrl;
}

class TrayCtrl : public QWidget
{
    Q_OBJECT

public:
    explicit TrayCtrl(QWidget *parent = nullptr);
    ~TrayCtrl();

    // 设置控件编号 - 用于区分上料、下料、OK、NG
    void setCtrlIndex(int index);

    // **************料盘**************
    // 初始化料盘模型数据
    void initTrayModel(int rows, int cols, QList<int> unUsedList = QList<int>());
    // 获取料盘行列
    void GetTrayRowCol(int& rows, int& cols);
    // 设置料盘名称
    void setTrayName(const QString& name, bool suffix = false);
    // 设置料盘单元格数据
    bool setTrayCellData(int row, int col, const QVariant &value, int role = Qt::UserRole);
    // 获取料盘单元格数据
    QVariant getTrayCellData(int row, int col, int role = Qt::UserRole);
    // 设置料盘单元格默认颜色
    void setTrayCellDefaultColor(const QColor& color);

    // **************料仓**************
    // 初始化料仓模型数据
    void initSilosModel(int rows);
    // 设置料仓名称
    void setSilosName(const QString& name);
    // 设置料仓单元格数据
    bool setSilosCellData(int row, const QVariant &value, int role = Qt::UserRole);
    // 获取料仓单元格数据
    QVariant getSilosCellData(int row, int role = Qt::UserRole);
    // 设置料仓单元格默认颜色
    void setSilosCellDefaultColor(const QColor& color);
    // 设置料仓层数信息
    void initSilosLayer();
    // 设置托盘信息
    void initTrayCell(QColor color, QList<int> unUsedList);
    void RenameCellName();
    void SetTrayAttribute(bool visible,bool bEdit=false);
    void SetSelectFlag(bool select);
    void ClearSelect();


    void SetSilosVisible(bool visible);

    QPixmap GrabTrayImg();

signals:
    // 选择料盘信号 - 编号，层号，行号，列好
    void sigTraySelected(int index, int silos, int row, int col);
    // 选择料仓信号 - 编号，层号
    void sigSilosSelected(int index, int silos);

private:
    Ui::TrayCtrl *ui;
    QStandardItemModel* m_pModelTray = nullptr;
    QStandardItemModel* m_pModelSilos = nullptr;
    int m_nIndex = -1;
    int m_nLayerIndex = -1;
    bool   m_flagTrayEdit = false;
};

#endif // TRAYCTRL_H
