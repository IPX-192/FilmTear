#ifndef MYQWTPLOT_H
#define MYQWTPLOT_H

#define QWT_DLL

//#define USE_OPENGL

#include <QWidget>
#include <qwt_plot.h>
#include <qwt_plot_layout.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_renderer.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_histogram.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_zoomer.h>
#include <qwt_plot_panner.h>
#include <qwt_plot_magnifier.h>
#include <qwt_legend.h>
#include <qwt_legend_label.h>
#include <qwt_column_symbol.h>
#include <qwt_curve_fitter.h>
#include <qwt_series_data.h>
#include <qpen.h>
#include <qwt_symbol.h>
#include <qwt_picker_machine.h>
#include <qevent.h>
#include <qwt_plot_glcanvas.h>
#include <qwt_plot_zoomer.h>
#include <qwt_plot_panner.h>
#include <qwt_plot_magnifier.h>

#ifdef USE_OPENGL
class GLCanvas: public QwtPlotGLCanvas
{
public:
    GLCanvas(QwtPlot *parent = nullptr):
        QwtPlotGLCanvas(parent)
    {
        setContentsMargins(1, 1, 1, 1);
    }

protected:
    virtual void paintEvent( QPaintEvent *event)
    {
        QPainter painter(this);
        painter.setClipRegion(event->region());

        QwtPlot *plot = qobject_cast<QwtPlot *>(parent());
        if (plot)
            plot->drawCanvas(&painter);


        painter.setPen(palette().windowText().color());
#if QT_VERSION >= 0x050000
        painter.drawRect(rect().adjusted(1, 1, -1, -1));  //画布外层矩形
#else
        painter.drawRect(rect().adjusted(0, 0, -1, -1));
#endif
    }
};
#endif

class MyPlotMagnifier: public QwtPlotMagnifier
{
    Q_OBJECT
public:
    explicit MyPlotMagnifier(QWidget *widget);
    ~MyPlotMagnifier();

protected:
    void rescale(double factor);
};

class MyQwtPlot: public QwtPlot
{
    Q_OBJECT

public:
    explicit MyQwtPlot(bool authority = false, QWidget *parent = nullptr);
    ~MyQwtPlot();

    /**
      *@brief 设置曲线的个数，调用后会删除已经存在的曲线，重新创建新的曲线
      *@param count 曲线的个数
      *@param curveName 曲线的名称
      *@param DotFlag true 只显示数据点。样本点不受影响，如果设置显示样本点，那么样本点依然会显示
      */
    void SetCurveCount(int count, const QStringList &curveName, bool DotFlag = false);
    /**
      *@brief 添加一个新曲线
      *@param curveName 曲线的名称
      *@param DotFlag true 只显示数据点。样本点不受影响，如果设置显示样本点，那么样本点依然会显示
      */
    void AppendCurve(const QString &curveName, bool DotFlag = false);

    /**
      *@brief 设置图表的标题
      *@param title 标题
      */
    void SetTitle(const QString &title);

    /**
      *@brief 设置曲线的颜色
      *@param curveIndex 设置的曲线序号
      *@param color 曲线颜色
      */
    void SetCurveColor(int curveIndex, const QColor &color);

    /**
      *@brief 设置曲线的大小
      *@param curveIndex 设置的曲线序号
      *@param size 曲线大小
      */
    void SetCurveSize(int curveIndex, int size);

    /**
      *@brief 设置曲线绘制的画笔，包括颜色和大小
      *@param curveIndex 设置的曲线序号
      *@param pen 曲线画笔
      */
    void SetCurvePen(int curveIndex, const QPen &pen);

    /**
      *@brief 是否显示样本点
      *@param curveIndex 需要显示样本点的曲线序号
      *@param flag true 显示 false 删除
      */
    void SetSymbolFlag(int curveIndex, bool flag);

    /**
      *@brief 设置曲线的点数据, 深拷贝数据
      *@param curveIndex 设置的曲线序号
      *@param points 点集
      */
    void SetValueDeepCopy(int curveIndex, const QVector<QPointF> &points);

    /**
      *@brief 设置曲线的点数据, 深拷贝数据
      *@param curveIndex 设置的曲线序号
      *@param xData X的数据点集合
      *@param yData Y的数据点集合
      *@param dataSize 数据点的数量
      */
    void SetValueDeepCopy(int curveIndex, double *xData, double *yData, int dataSize);

    /**
      *@brief 设置曲线的点数据,浅拷贝可以提升效率降低内存使用,但是很危险，需要外部确保在曲线存在期间数据不能被释放
      *@param curveIndex 设置的曲线序号
      *@param xData X的数据点集合
      *@param yData Y的数据点集合
      *@param dataSize 数据点的数量
      */
    void SetValueSimpleCopy(int curveIndex, double *xData, double *yData, int dataSize);

    /**
      *@brief 设置画布的调色板
      *@param palette 调色板类
      */
    void SetCanvasPalette(const QPalette &palette);

    /**
      *@brief 设置轴的字体
      *@param axis 设置的轴
      *@param font 字体样式
      */
    void SetAxisFont(QwtPlot::Axis axis, const QFont &font);

    /**
      *@brief 设置刻度线的名称
      *@param axis 设置的轴
      *@param title 刻度线名称
      */
    void SetAxisTitle(QwtPlot::Axis axis, const QString &title);

    /**
      *@brief 设置刻度的最小、大以及步进值, 不设置为自动根据值范围进行调整
      *@param axis 设置的轴
      *@param min 最小值
      *@param max 最大值
      *@param stepSize 值范围的步进值
      */
    void SetAxisScale(QwtPlot::Axis axis, double min, double max, double stepSize = 0);

    /**
      *@brief 设置刻度线刻度值自动变化
      *@param axis 自动变化的轴
      *@param flag true 自动变化
      */
    void SetAxisAutoScale(QwtPlot::Axis axis, bool flag = true);

    /**
      *@brief 设置最小刻度或者主刻度的最大刻度数量
      *@param axis 设置的轴
      *@param MinorFlag true 最小刻度， false 主刻度
      *@param maxValue 最大值
      */
    void SetAxisMaxMinorOrMajor(QwtPlot::Axis axis, bool MinorFlag = true, int maxValue = 1);

    /**
      *@brief 是否显示图例, 图例现在显示的是曲线的颜色，也可以显示其它
      *@param flag false 不显示
      */
    void SetLegendsEnable(bool flag = false);

    /**
      *@brief 返回X轴的最小值
      *@param curveIndex 曲线序号
      */
    double GetMinXValue(int curveIndex);

    /**
      *@brief 返回X轴的最大值
      *@param curveIndex 曲线序号
      */
    double GetMaxXValue(int curveIndex);

    /**
      *@brief 返回Y轴的最小值
      *@param curveIndex 曲线序号
      */
    double GetMinYValue(int curveIndex);

    /**
      *@brief 返回Y轴的最大值
      *@param curveIndex 曲线序号
      */
    double GetMaxYValue(int curveIndex);
    /**
      *@brief 返回曲线个数
      */
    int GetCurveCount();
    /**
      *@brief 返回曲线个数
      */
    void CleanCurve();

    /**
      *@brief 导出曲线图到文件
      *@param name 导出的文档名称
      *@param fileDialog 自动弹出文件对话框用于选择保存路径和文件名，默认 false不弹出
      *@param sizeMM 文档的大小以mm为单位
      *@param resolution 分辨率，单位为每英寸点数（dpi）
      */
    void ExportPlot(const QString &name, bool fileDialog = false,
                    const QSizeF &sizeMM = QSizeF(300, 200), int resolution = 85);

    /**
      *@brief 导出曲线图到绘图设备上
      *@param device 绘图设备
      */
    void ExportToPainter(QPaintDevice &device);

    /**
      *@brief 获取曲线绘制的画笔
      *@param curveIndex 曲线序号
      *@return  曲线画笔
      */
    QPen GetCurvePen(int curveIndex);

private:
    /**
      *@brief 初始化
      */
    void Init();
    
private slots:
    /** 
      *@brief 点击图例，显示相应的曲线
      *@param itemInfo 具体的图例项目
      *@param on false 不显示曲线
      */
    void showItem(const QVariant &itemInfo, bool on); 

private:
#ifndef USE_OPENGL
    QwtPlotCanvas *m_canvas = nullptr;            //画布类
#else
    QwtPlotGLCanvas *m_canvas = nullptr;          //画布类
#endif
    QwtPlotGrid *m_plotGrid = nullptr;            //网格类
    QVector<QwtPlotCurve *> m_curve;              //曲线集合
    QwtLegend *m_legend = nullptr;                //图例类
    QwtPlotZoomer *m_zoomer = nullptr;
    MyPlotMagnifier *m_magnifier = nullptr;
    QwtPlotPanner *m_panner = nullptr;
    bool m_authority = false;
    QVector<QColor> m_color;                     //已经使用的颜色
};

#endif // MYQWTPLOT_H
