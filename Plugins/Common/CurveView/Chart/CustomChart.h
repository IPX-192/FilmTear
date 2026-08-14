#ifndef CUSTOMCHART_H
#define CUSTOMCHART_H

#include <QVector>
#include <QColor>
#include <QList>
#include <QMouseEvent>
#include "qcustomplot.h"

class CustomChart : public QCustomPlot
{
    Q_OBJECT

public:
    explicit CustomChart(QWidget *parent = nullptr);

    /**
     * @brief 设置图表标题
     * @param title 图表标题
     */
    void setChartTitle(QString title);
    /**
     * @brief 设置曲线X坐标范围
     * @param start X轴起点
     * @param end X轴终点
     */
    void setAxisXRange(double start, double end);
    /**
     * @brief 设置曲线坐标范围
     * @param start Y轴起点
     * @param end Y轴终点
     */
    void setAxisYRange(double start, double end);
    /**
     * @brief 设置显示图例
     */
    void setLegendVisible(bool show);
    /**
    * @brief 设置鼠标交互功能开启
    */
    void setInteractions(bool enable);
    /**
     * @brief 添加一条曲线
     * @param name 曲线名称（显示在图例中）
     * @param xData X轴数据
     * @param yData Y轴数据
     */
    void addCurve(const QString &name, const QVector<double> &xData, const QVector<double> &yData);
    /**
     * @brief 添加数据点
     * @param name 曲线名称（显示在图例中）
     * @param xData X轴数据
     * @param yData Y轴数据
     */
    void addCurve(const QString &name, double xData, double yData);

    /**
     * @brief 清空所有曲线
     */
    void clearAllCurves();

    /**
     * @brief 添加N个柱状图,带标签传入
     * @param name 柱状图名称
     * @param label X轴文字标签
     * @param data  XY轴数据
     */
    void addBars(const QString& name, const QVector<QString>& labels, const QVector<double>& xData, const QVector<double>& yData);

    /**
     * @brief 设置文本标签
     * @param name 柱状图名称
     * @param labels  X轴标签
     * @param xData   X轴坐标
     */
    void setTextTicker(const QString& name, QVector<QString>& labels,const QVector<double>& xData);

    /**
     * @brief 设置柱状图数据
     * @param name 柱状图名称
     * @param data  XY轴数据
     */
    void addBars(const QString& name, const QVector<double>& xData, const QVector<double>& yData);

    /**
     * @brief 添加单点柱状图数据
     * @param name 曲线名称（显示在图例中）
     * @param xData X轴数据
     * @param yData Y轴数据
     */
    void addBar(const QString &name,QString label, QPointF data);

    /**
     * @brief 清空所有柱状图
     */
    void clearAllBars();

protected:
    //创建曲线
    QCPGraph * creatGraph(QString name);
    //创建柱状图
    QCPBars * creatBar(QString name);
    //创建位置文本标签
    void  creatPosText(QString name,double xpos,double ypos);
    // 设置主题背景
    void setChartTheme();
    // 获取下一个颜色
    QColor getNextColor();
    // 初始化图表外观和交互设置
    void  setupPlot();
    //鼠标双击还原
    void  mouseDoubleClickEvent(QMouseEvent * event);
    //自适应Y坐标
    void RescalePlot();

private:
    // 预定义的颜色列表，用于自动循环分配
    QList<QColor> m_colorList;
    // 当前使用的颜色索引
    int m_colorIndex;
    //名称对应的曲线
    QMap<QString,QCPGraph * >m_graph;
    QMap<QString,QCPBars * >m_bars;
    QMap<QString, QMap<double, QCPItemText*>>m_posTextLabel;
    QPointF m_axisRangeX;
    QPointF m_axisRangeY;
    bool   m_interactions = false;  //交互标志


private slots:
    // 用于同步图例和曲线选中状态的槽函数
    void syncSelection();
};

#endif // CUSTOMCHART_H
