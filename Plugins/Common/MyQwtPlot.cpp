#include "MyQwtPlot.h"

MyQwtPlot::MyQwtPlot(bool authority, QWidget *parent)
    :QwtPlot(parent),
      m_authority(authority)
{
    Init();
}

MyQwtPlot::~MyQwtPlot()
{
    if (m_plotGrid)
        delete m_plotGrid;
    if (m_legend)
        delete m_legend;
    if (m_zoomer)
        delete m_zoomer;
    if (m_magnifier)
        delete m_magnifier;
    if (m_panner)
        delete m_panner;
    for (int i = 0; i < m_curve.count(); i++)
    {
        if (m_curve[i])
        {
            m_curve[i]->detach();
            delete m_curve[i];
            m_curve[i] = nullptr;
        }
    }
    m_curve.clear();
}

void MyQwtPlot::SetCurveCount(int count, const QStringList &curveName, bool DotFlag)
{
    //先释放以前的
    for (int i = 0; i < m_curve.count(); i++)
    {
        m_curve[i]->detach();
        delete m_curve[i];
        m_curve[i] = nullptr;
    }
    m_curve.clear();
    for (int i = 0; i < count; i++)
    {
        m_curve.push_back(new QwtPlotCurve);
        if (i < curveName.count())
            m_curve[i]->setTitle(curveName[i]);

        if (DotFlag)
            m_curve[i]->setStyle(QwtPlotCurve::Dots);
        else
            m_curve[i]->setStyle(QwtPlotCurve::Lines);
        //默认pen样式
        QPen pen;
        pen.setCapStyle(Qt::RoundCap);
        pen.setJoinStyle(Qt::RoundJoin);
        QColor color = QColor::fromHsl(rand() % 360, rand() % 256, rand() % 200);
        //判断随机颜色是否已经存在, 最大随机次数，防止一直循环
        int cycleIndex = 0, cycleCount = 50;
        for (int i = 0; i < m_color.count(); i++)
        {
            if (color == m_color[i])
            {
                ++cycleIndex;
                i = 0;
                color = QColor::fromHsl(rand() % 360, rand() % 256, rand() % 200);
                if (cycleIndex > cycleCount)
                    break;
                else
                    continue;
            }
        }
        m_color.append(color);
        pen.setColor(color);  //随机颜色
        pen.setWidth(2);
        m_curve[i]->setPen(pen);
        //多线程渲染
        m_curve[i]->setRenderThreadCount(0);
        m_curve[i]->setRenderHint(QwtPlotItem::RenderAntialiased, true);//线条光滑化
        m_curve[i]->setLegendAttribute(m_curve[i]->LegendShowLine);//显示图例的标志，这里显示线的颜色。
        m_curve[i]->attach(this);
    }
}

void MyQwtPlot::AppendCurve(const QString &curveName, bool DotFlag)
{
    m_curve.push_back(new QwtPlotCurve);
    int index = m_curve.size() - 1;
    if (index < 0)
        index = 0;
    m_curve[index]->setTitle(curveName);

    if (DotFlag)
        m_curve[index]->setStyle(QwtPlotCurve::Dots);
    else
        m_curve[index]->setStyle(QwtPlotCurve::Lines);
    //默认pen样式
    QPen pen;
    pen.setCapStyle(Qt::RoundCap);
    pen.setJoinStyle(Qt::RoundJoin);
    QColor color = QColor::fromHsl(rand() % 360, rand() % 256, rand() % 200);
    //判断随机颜色是否已经存在, 最大随机次数，防止一直循环
    int cycleIndex = 0, cycleCount = 50;
    for (int i = 0; i < m_color.count(); i++)
    {
        if (color == m_color[i])
        {
            ++cycleIndex;
            i = 0;
            color = QColor::fromHsl(rand() % 360, rand() % 256, rand() % 200);
            if (cycleIndex > cycleCount)
                break;
            else
                continue;
        }
    }
    m_color.append(color);
    pen.setColor(color);  //随机颜色
    pen.setWidth(2);
    m_curve[index]->setPen(pen);
    //多线程渲染
    m_curve[index]->setRenderThreadCount(0);
    m_curve[index]->setRenderHint(QwtPlotItem::RenderAntialiased, true);    //线条光滑化
    m_curve[index]->setLegendAttribute(m_curve[index]->LegendShowLine); //显示图例的标志，这里显示线的颜色。
    m_curve[index]->attach(this);
}

void MyQwtPlot::SetTitle(const QString &title)
{
    setTitle(title);
    replot();
}

void MyQwtPlot::SetCurveColor(int curveIndex, const QColor &color)
{
    if (m_curve.count() > curveIndex)
    {
        QPen pen = m_curve[curveIndex]->pen();
        pen.setColor(color);
        m_curve[curveIndex]->setPen(pen);
        replot();
    }
}

void MyQwtPlot::SetCurveSize(int curveIndex, int size)
{
    if (m_curve.count() > curveIndex)
    {
        QPen pen = m_curve[curveIndex]->pen();
        pen.setWidth(size);
        m_curve[curveIndex]->setPen(pen);
        replot();
    }
}

void MyQwtPlot::SetCurvePen(int curveIndex, const QPen &pen)
{
    if (m_curve.count() > curveIndex)
    {
        m_curve[curveIndex]->setPen(pen);
        replot();
    }
}

void MyQwtPlot::SetSymbolFlag(int curveIndex, bool flag)
{
    if (m_curve.count() > curveIndex)
    {
        if (flag)
        {
            if (m_curve[curveIndex]->symbol())
            {
                delete m_curve[curveIndex]->symbol();
            }
            //设置样本点的样式、颜色、大小
            QwtSymbol *symbol = new QwtSymbol(QwtSymbol::Ellipse,
                                              QBrush(Qt::yellow),
                                              QPen(Qt::red, 2),
                                              QSize(6, 6));
            m_curve[curveIndex]->setSymbol(symbol);
        }
        else
        {
            if (m_curve[curveIndex]->symbol())
            {
                delete m_curve[curveIndex]->symbol();
                m_curve[curveIndex]->setSymbol(nullptr);
            }
        }
        replot();
    }
}

void MyQwtPlot::SetValueDeepCopy(int curveIndex, const QVector<QPointF> &points)
{
    if (m_curve.count() > curveIndex)
    {
        m_curve[curveIndex]->setPaintAttribute(QwtPlotCurve::ImageBuffer);
        m_curve[curveIndex]->setSamples(points);
        if (m_authority)
        {
            //需要重新设置缩放的原始画布
            if (m_zoomer)
            {
                m_zoomer->zoom(m_zoomer->zoomBase());
                delete m_zoomer;
            }
            m_zoomer = new QwtPlotZoomer(m_canvas);        //选择图形局部放大
            //勾选扩大区域边界颜色
            m_zoomer->setRubberBandPen(QColor(Qt::red));
            //ctrl+右键==回复到原始状态
            m_zoomer->setMousePattern(QwtEventPattern::MouseSelect2, Qt::RightButton, Qt::ControlModifier);
            //右键==恢复到上一次扩大之前的状态
            m_zoomer->setMousePattern(QwtEventPattern::MouseSelect3, Qt::RightButton);
        }

        replot();
    }
}

void MyQwtPlot::SetValueDeepCopy(int curveIndex, double *xData, double *yData, int dataSize)
{
    if (m_curve.count() > curveIndex)
    {
        m_curve[curveIndex]->setPaintAttribute(
                    QwtPlotCurve::ImageBuffer, dataSize > 1000);
        m_curve[curveIndex]->setSamples(xData, yData, dataSize);
        if (m_authority)
        {
            //需要重新设置缩放的原始画布
            if (m_zoomer)
            {
                m_zoomer->zoom(m_zoomer->zoomBase());
                delete m_zoomer;
            }
            m_zoomer = new QwtPlotZoomer(m_canvas);        //选择图形局部放大
            //勾选扩大区域边界颜色
            m_zoomer->setRubberBandPen(QColor(Qt::red));
            //ctrl+右键==回复到原始状态
            m_zoomer->setMousePattern(QwtEventPattern::MouseSelect2, Qt::RightButton, Qt::ControlModifier);
            //右键==恢复到上一次扩大之前的状态
            m_zoomer->setMousePattern(QwtEventPattern::MouseSelect3, Qt::RightButton);
        }
        replot();
    }
}

void MyQwtPlot::SetValueSimpleCopy(int curveIndex, double *xData, double *yData, int dataSize)
{
    if (m_curve.count() > curveIndex)
    {
        m_curve[curveIndex]->setPaintAttribute(
                    QwtPlotCurve::ImageBuffer, dataSize > 1000);
        m_curve[curveIndex]->setRawSamples(xData, yData, dataSize);
        if (m_authority)
        {
            //需要重新设置缩放的原始画布
            if (m_zoomer)
            {
                m_zoomer->zoom(m_zoomer->zoomBase());
                delete m_zoomer;
            }
            m_zoomer = new QwtPlotZoomer(m_canvas);        //选择图形局部放大
            //勾选扩大区域边界颜色
            m_zoomer->setRubberBandPen(QColor(Qt::red));
            //ctrl+右键==回复到原始状态
            m_zoomer->setMousePattern(QwtEventPattern::MouseSelect2, Qt::RightButton, Qt::ControlModifier);
            //右键==恢复到上一次扩大之前的状态
            m_zoomer->setMousePattern(QwtEventPattern::MouseSelect3, Qt::RightButton);
        }
        replot();
    }
}

void MyQwtPlot::SetCanvasPalette(const QPalette &palette)
{
    m_canvas->setPalette(palette);
    replot();
}

void MyQwtPlot::SetAxisFont(QwtPlot::Axis axis, const QFont &font)
{
    setAxisFont(axis, font);
    replot();
}

void MyQwtPlot::SetAxisTitle(QwtPlot::Axis axis, const QString &title)
{
    setAxisTitle(axis, title);
    replot();
}

void MyQwtPlot::SetAxisScale(QwtPlot::Axis axis, double min, double max, double stepSize)
{
    setAxisScale(axis, min, max, stepSize);
    if (m_authority)
    {
        //需要重新设置缩放的原始画布
        if (m_zoomer)
        {
            m_zoomer->zoom(m_zoomer->zoomBase());
            delete m_zoomer;
        }
        m_zoomer = new QwtPlotZoomer(m_canvas);        //选择图形局部放大
        //勾选扩大区域边界颜色
        m_zoomer->setRubberBandPen(QColor(Qt::red));
        //ctrl+右键==回复到原始状态
        m_zoomer->setMousePattern(QwtEventPattern::MouseSelect2, Qt::RightButton, Qt::ControlModifier);
        //右键==恢复到上一次扩大之前的状态
        m_zoomer->setMousePattern(QwtEventPattern::MouseSelect3, Qt::RightButton);
    }
    replot();
}

void MyQwtPlot::SetAxisAutoScale(QwtPlot::Axis axis, bool flag)
{
    setAxisAutoScale(axis, flag);
    replot();
}

void MyQwtPlot::SetAxisMaxMinorOrMajor(QwtPlot::Axis axis, bool MinorFlag, int maxValue)
{
    if (MinorFlag)
        setAxisMaxMinor(axis, maxValue);
    else
        setAxisMaxMajor(axis, maxValue);
    replot();
}

void MyQwtPlot::SetLegendsEnable(bool flag)
{
    if (flag)
    {
        //-----------------图例设置----------------------//
        m_legend = new QwtLegend;
        m_legend->setDefaultItemMode(QwtLegendData::Checkable); //图例可被点击
        insertLegend(m_legend, QwtPlot::RightLegend);
        connect(m_legend, SIGNAL(checked( const QVariant &, bool, int )),
                SLOT(showItem(const QVariant &, bool)));//点击图例操作

        //生成图例
        //获取画了多少条曲线,如果为获取其他形状，注意改变参数
        QwtPlotItemList items = itemList(QwtPlotItem::Rtti_PlotCurve);
        for (int i = 0; i < items.size(); i++)
        {
            const QVariant itemInfo = itemToInfo(items[i]);
            QwtLegendLabel *legendLabel = qobject_cast<QwtLegendLabel *>(m_legend->legendWidget(itemInfo));
            if (legendLabel)
                legendLabel->setChecked(true);
            items[i]->setVisible(true);
        }
        replot();
    }
    else
    {
        insertLegend(nullptr);
        delete m_legend;
        m_legend = nullptr;
    }
}

double MyQwtPlot::GetMinXValue(int curveIndex)
{
    if (m_curve.count() > curveIndex)
    {
        return m_curve[curveIndex]->minXValue();
    }
    else
    {
        return 0;
    }
}

double MyQwtPlot::GetMaxXValue(int curveIndex)
{
    if (m_curve.count() > curveIndex)
    {
        return m_curve[curveIndex]->maxXValue();
    }
    else
    {
        return 0;
    }
}

double MyQwtPlot::GetMinYValue(int curveIndex)
{
    if (m_curve.count() > curveIndex)
    {
        return m_curve[curveIndex]->minYValue();
    }
    else
    {
        return 0;
    }
}

double MyQwtPlot::GetMaxYValue(int curveIndex)
{
    if (m_curve.count() > curveIndex)
    {
        return m_curve[curveIndex]->maxYValue();
    }
    else
    {
        return 0;
    }
}

int MyQwtPlot::GetCurveCount()
{
    return m_curve.size();
}

void MyQwtPlot::CleanCurve()
{
    //先释放以前的
    for (int i = 0; i < m_curve.count(); i++)
    {
        SetValueDeepCopy(i, QVector<QPointF>());
        m_curve[i]->detach();
        delete m_curve[i];
        m_curve[i] = nullptr;
    }
    m_curve.clear();
    m_color.clear();
}

void MyQwtPlot::ExportPlot(const QString &name, bool fileDialog, const QSizeF &sizeMM, int resolution)
{
    QwtPlotRenderer renderer;
    if (fileDialog)
        renderer.exportTo(this, name, sizeMM, resolution);
    else
        renderer.renderDocument(this, name, sizeMM, resolution);
}

void MyQwtPlot::ExportToPainter(QPaintDevice &device)
{
    QwtPlotRenderer renderer;
    renderer.renderTo(this, device);
}

QPen MyQwtPlot::GetCurvePen(int curveIndex)
{
    if (m_curve.count() > curveIndex)
    {
        return m_curve[curveIndex]->pen();
    }
    else
    {
        return QPen();
    }
}

void MyQwtPlot::Init()
{
    //----------------设置画布-------------------//
#ifdef USE_OPENGL
    m_canvas = new GLCanvas(this);
    m_canvas->setPalette(Qt::white);
    setCanvas(m_canvas);
#else
    m_canvas = new QwtPlotCanvas(this);
    m_canvas->setFrameStyle(QFrame::NoFrame);
    m_canvas->setLineWidth(1);
    m_canvas->setPalette(Qt::white);
    //圆角
    m_canvas->setBorderRadius(0);
    setCanvas(m_canvas);
    //绘制双缓冲区，尽可能重用pixmap缓冲区的内容。
    m_canvas->setPaintAttribute(QwtPlotCanvas::BackingStore, true);
    //replot调用的是update还是repaint
    //m_canvas->setPaintAttribute(QwtPlotCanvas::ImmediatePaint, true);
#endif
    plotLayout()->setAlignCanvasToScales(true);
    //    plotLayout()->setCanvasMargin(5, QwtPlot::yLeft);
    //    plotLayout()->setCanvasMargin(5, QwtPlot::xBottom);
    //    plotLayout()->setCanvasMargin(5, QwtPlot::yRight);
    //    plotLayout()->setCanvasMargin(5, QwtPlot::xTop);

    //----------------设置栅格线-------------------//
    m_plotGrid = new QwtPlotGrid;
    m_plotGrid->enableX(true);
    m_plotGrid->enableY(true);
    m_plotGrid->setMajorPen(QColor(128, 128, 128), 1, Qt::DotLine);
    //m_plotGrid->setItemAttribute(QwtPlotItem::Margins, true);
    m_plotGrid->attach(this);

    //-----------------刻度线设置----------------------//
    //this->setAxisTitle(QwtPlot::yLeft, "k");
    //this->setAxisLabelAlignment(QwtPlot::yLeft, Qt::AlignCenter);
    //大刻度的数量，如值范围1-10之间有多少个大刻度
//    this->setAxisMaxMajor(QwtPlot::xBottom, 12);
//    this->setAxisMaxMajor(QwtPlot::yLeft, 4);
    //最小刻度的数量，如1-2之间有多少个刻度
    //this->setAxisMaxMinor(QwtPlot::yLeft, 1);
    //设置刻度的最小、大以及步进值, 不设置为自动根据值范围进行调整
    //this->setAxisScale(QwtPlot::yLeft, -0.3, 0.3);
   // this->setAxisMaxMinor(QwtPlot::xBottom, 1);
    //    this->setAxisScale(QwtPlot::xBottom, 0, 10, 1);
    QFont font(QString::fromLocal8Bit("黑体"), 11);
    QwtText text;
    text.setFont(font);
    this->setAxisTitle(QwtPlot::yLeft, text);
    this->setAxisTitle(QwtPlot::xBottom, text);

    //-----------------缩放设置----------------------//
    if (m_authority)
    {
        m_zoomer = new QwtPlotZoomer(m_canvas);        //选择图形局部放大
        //勾选扩大区域边界颜色
        m_zoomer->setRubberBandPen(QColor(Qt::red));
        //ctrl+右键==回复到原始状态
        m_zoomer->setMousePattern(QwtEventPattern::MouseSelect2, Qt::RightButton, Qt::ControlModifier);
        //右键==恢复到上一次扩大之前的状态
        m_zoomer->setMousePattern(QwtEventPattern::MouseSelect3, Qt::RightButton);

        //默认的左键移动功能
        m_panner = new QwtPlotPanner(m_canvas);
        //设置哪个按钮移动画布  如果不设置(注释掉当前行)按钮默认为左键为移动
        m_panner->setMouseButton(Qt::MidButton);

        //默认的滑轮及右键缩放功能  图形的整体缩放
        m_magnifier = new MyPlotMagnifier(m_canvas);
        //设置哪个按钮与滑轮为缩放画布 如果不设置(注释掉当前行)按钮默认为滑轮以及右键为缩放
        m_magnifier->setMouseButton(Qt::NoButton);
    }

    setAutoReplot(false); //设置是否自动重画，自动比较耗时
}

void MyQwtPlot::showItem(const QVariant &itemInfo, bool on)
{
    QwtPlotItem *plotItem = infoToItem(itemInfo);
    if (plotItem)
    {
        plotItem->setVisible(on);
        replot();
    }
}

MyPlotMagnifier::MyPlotMagnifier(QWidget *widget):
    QwtPlotMagnifier(widget)
{

}

MyPlotMagnifier::~MyPlotMagnifier()
{

}

void MyPlotMagnifier::rescale(double factor)
{
    QwtPlot* plt = plot();
    if (plt == nullptr)
        return;

    factor = qAbs(factor);
    if (factor == 1.0 || factor == 0.0)
        return;

    bool doReplot = false;

    const bool autoReplot = plt->autoReplot();
    plt->setAutoReplot(false);

    QPoint cursorPos = canvas()->mapFromGlobal(canvas()->cursor().pos());

    for (int axisId = 0; axisId < QwtPlot::axisCnt; axisId++)
    {
        if (isAxisEnabled(axisId))
        {
            const QwtScaleMap scaleMap = plt->canvasMap(axisId);

            double v1 = scaleMap.s1();
            double v2 = scaleMap.s2();

            if (scaleMap.transformation())
            {
                //绘制设备的坐标系始终是线性的
                v1 = scaleMap.transform( v1 ); // scaleMap.p1()
                v2 = scaleMap.transform( v2 ); // scaleMap.p2()
            }

            const double posInAxis = (axisId / 2 ? cursorPos.x() : cursorPos.y());
            const double lenthInAxis = (axisId / 2 ? canvas()->rect().width() : canvas()->rect().height());
            double posRadio = posInAxis / lenthInAxis;//获取当前鼠标在X/Y轴的位置（百分比）
            if(axisId / 2 == 0)//plot的Y轴方向与鼠标Y轴方向不同
            {
                posRadio = 1 - posRadio;
            }
            double center = posRadio * (v2 - v1) + v1;//将百分比转化为v1到v2的点
            v1 = center - (center - v1) * factor;//将鼠标中心分成两部分，分别进行缩放
            v2 = center + (v2 - center) * factor;

            if (scaleMap.transformation())
            {
                v1 = scaleMap.invTransform(v1);
                v2 = scaleMap.invTransform(v2);
            }

            plt->setAxisScale(axisId, v1, v2);
            doReplot = true;
        }
    }

    plt->setAutoReplot(autoReplot);

    if (doReplot)
        plt->replot();
}
