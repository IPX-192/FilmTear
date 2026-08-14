#include "CustomChart.h"

CustomChart::CustomChart(QWidget *parent)
    : QCustomPlot(parent)
    , m_colorIndex(0)
{
    // 初始化颜色列表 (使用一些对比度高的颜色)
    m_colorList << QColor(30, 144, 255)   // DodgerBlue
                << QColor(255, 69, 0)     // RedOrange
                << QColor(50, 205, 50)    // LimeGreen
                << QColor(255, 215, 0)    // Gold
                << QColor(138, 43, 226)   // BlueViolet
                << QColor(0, 206, 209)    // DarkTurquoise
                << QColor(255, 20, 147);  // DeepPink

    m_axisRangeX=QPointF(0,100);
    m_axisRangeY=QPointF(0,100);

    //setChartTheme();
    setupPlot();
}

void CustomChart::setChartTitle(QString title)
{
	QCPTextElement* m_title = new QCPTextElement(this, title);
	m_title->setMargins(QMargins(0, 9, 0, 0));
	m_title->setTextColor(QColor(0, 0, 0));
	plotLayout()->insertRow(0);
	plotLayout()->addElement(0, 0, m_title);
	plotLayout()->setRowSpacing(0);     //设置行间距
}

void CustomChart::setupPlot()
{
    // 2. --- 新增：设置图例标签的样式 ---
    QFont legendFont = font();
    legendFont.setPointSize(9);
    this->legend->setFont(legendFont);

    // 设置图例被选中时的字体（例如：变粗、变红）
    QFont selectedFont = legendFont;
    selectedFont.setBold(true);
    this->legend->setSelectedFont(selectedFont);
    this->legend->setSelectedTextColor(Qt::red); // 选中时文字变红，提示更明显

    // 关键点：允许选择图例中的具体项目（Item），而不仅仅是整个图例框
    this->legend->setSelectableParts(QCPLegend::spItems);

    // 3. 坐标轴设置（保持不变）
    //this->xAxis->setLabel(u8"位置");
    //this->yAxis->setLabel(u8"分数");
    //this->selectionRect()->setMode(QCP::srmNone);

    // 4. --- 新增：连接选中改变信号到同步槽函数 ---
    // 当用户点击界面导致选中状态变化时，触发 syncSelection
    connect(this, &QCustomPlot::selectionChangedByUser, this, &CustomChart::syncSelection);
}

QColor CustomChart::getNextColor()
{
    if (m_colorList.isEmpty()) return Qt::black;
    QColor color = m_colorList[m_colorIndex % m_colorList.size()];
    m_colorIndex++;
    return color;
}

// --- 新增：核心同步逻辑 ---
void CustomChart::syncSelection()
{
    // 遍历所有曲线
    for (int i = 0; i < this->graphCount(); ++i)
    {
        QCPGraph *graph = this->graph(i);
        if (!graph) continue;

        // 获取该曲线对应的图例项
        QCPPlottableLegendItem *item = this->legend->itemWithPlottable(graph);

        // 1. 判断当前状态
        // 只要 曲线被选中 OR 图例被选中，我们就认为用户想选中这一组
        bool graphSelected = !graph->selection().isEmpty();
        bool legendSelected = item->selected();

        // 2. 执行同步（互斥模式）
        // 如果使用了 QCP::srmNone (单选模式)，我们通常希望点击空白处取消所有
        // 这里实现了：如果这一组里有任意一个被选中，则把两个都设为选中；否则都取消。

        if (graphSelected || legendSelected) {
            // 只有当状态不一致时才去设置，避免重复刷新
            if (!graphSelected) {
                // 选中整条曲线
               // QCPDataSelection selection = graph->data()->selection(); // 获取全选范围
                // 注意：这里简单起见，我们选中所有数据点。
                // 如果只想高亮而不选具体点，可以用 setSelection(QCPDataSelection(graph->data()->dataRange()));
                graph->setSelection(QCPDataSelection(graph->data()->dataRange()));
            }
            if (!legendSelected) {
                item->setSelected(true);
            }
        } else {
            // 如果两个都没被用户直接选中（说明用户点击了其他地方），确保它俩都是非选中状态
            // 这一步对于“点击空白处取消选择”很重要
             if (!graph->selection().isEmpty()) graph->setSelection(QCPDataSelection());
             if (item->selected()) item->setSelected(false);
        }
    }
}

void CustomChart::setAxisYRange(double start, double end)
{
    yAxis->setRange(start, end);
    m_axisRangeY = QPointF(start, end);
    yAxis->ticker()->setTickOrigin(start);
    replot();
}

void CustomChart::setLegendVisible(bool show)
{
    this->legend->setVisible(show);
}

void CustomChart::setInteractions(bool enable)
{
    if (enable) {
		// 1. 开启交互功能：
        // 1. 基础交互设置（保持不变）
		this->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom |
			QCP::iSelectPlottables | QCP::iSelectLegend);
    }
    m_interactions = enable;
}

void CustomChart::setAxisXRange(double start, double end)
{
    xAxis->setRange(start, end);
    m_axisRangeX = QPointF(start, end);
    xAxis->ticker()->setTickOrigin(start);
    replot();
}

void CustomChart::addCurve(const QString &name, const QVector<double> &xData, const QVector<double> &yData)
{
    QCPGraph *graph;
    // 1. 创建新 Graph
    if(m_graph.contains(name))
        graph=m_graph[name];
    else{
       graph=creatGraph(name);
    }
    graph->setData(xData, yData);

    // 5. 自动调整坐标轴范围以显示新数据
    this->rescaleAxes();
    this->replot();
}

void CustomChart::addCurve(const QString &name, double xData, double yData)
{
    QCPGraph *graph;
    // 1. 创建新 Graph
    if(m_graph.contains(name))
        graph=m_graph[name];
    else{
       graph=creatGraph(name);
    }

    graph->addData(xData, yData);
    // 5. 自动调整坐标轴范围以显示新数据
    //this->rescaleAxes();
    this->replot();
}

void CustomChart::clearAllCurves()
{
    this->clearGraphs();
    m_colorIndex = 0; // 重置颜色循环
    this->replot();
}

void CustomChart::addBars(const QString &name, const QVector<QString>&labels, const QVector<double> & xData, const QVector<double>& yData)
{
    QCPBars *bar;
    // 1. 创建新 Graph
    if(m_bars.contains(name))
        bar=m_bars[name];
    else{
       bar=creatBar(name);
    }
	// 3. 将数据设置给柱状图
    bar->setData(xData, yData);
    // 默认 X 轴显示的是数字 1,2,3... 我们需要把它映射成标签.
	QSharedPointer<QCPAxisTickerText> textTicker(new QCPAxisTickerText);
	textTicker->addTicks(xData, labels);
	xAxis->setTicker(textTicker);
	// 遍历数据，为每个柱子添加数值标签
	for (int i = 0; i < xData.size(); i++)
	{
		double key = xData[i];
		double value = yData[i];
        creatPosText(name, key, value);
	}
    //this->rescaleAxes();
    //this->replot();
    RescalePlot();
}

void CustomChart::setTextTicker(const QString& name, QVector<QString>& labels, const QVector<double>& xData)
{
	QSharedPointer<QCPAxisTickerText> textTicker(new QCPAxisTickerText);
	textTicker->addTicks(xData, labels);
    xAxis->setTicker(textTicker);
}

void CustomChart::addBars(const QString& name, const QVector<double>& xData, const QVector<double>& yData)
{
	QCPBars* bar;
	// 1. 创建新 Graph
	if (m_bars.contains(name))
		bar = m_bars[name];
	else {
		bar = creatBar(name);
	}
	// 3. 将数据设置给柱状图
	bar->setData(xData, yData);
	
	// 遍历数据，为每个柱子添加数值标签
	for (int i = 0; i < xData.size(); i++)
	{
		double key = xData[i];
		double value = yData[i];
		if (value == 0)continue;
        creatPosText(name, key, value);
	}
	//this->rescaleAxes();
	//this->replot();
	RescalePlot();
}

void CustomChart::addBar(const QString& name, QString label, QPointF data)
{
    QCPBars *bar;
    // 1. 创建新 Graph
    if(m_bars.contains(name))
        bar=m_bars[name];
    else{
       bar=creatBar(name);
    }
    bar->addData(data.x(), data.y());
    creatPosText(name, data.x(), data.y());
    // 5. 自动调整坐标轴范围以显示新数据
    //this->rescaleAxes();
    //this->replot();
    RescalePlot();
}

void CustomChart::RescalePlot()
{
	// 1. 先自适应 Y 轴
    yAxis->rescale(true);
	// 2. 获取刚才自动计算出的范围
	QCPRange currentRange = yAxis->range();

	// 3. 调整上界：增加 10%~20% 的空间
	// 这样柱子上方就有空隙显示数值标签了
	double newUpper = currentRange.upper * 1.1; // 乘以 1.1 代表增加 10% 高度
	// 如果有负数数据，或者最小值不为0，建议用这种方式扩充范围：
	// ui->customPlot->yAxis->scaleRange(1.1); 

	// 4. 应用新范围（保持下界不变，通常柱状图下界是0）
	 yAxis->setRangeUpper(newUpper);
	 replot();
}

void CustomChart::clearAllBars()
{
 //   for(auto item: m_bars)
	//	removePlottable(item);
	//// 在柱子上添加了文字标签(QCPItemText)，也需要清空
 //   for (auto& item : m_posTextLabel) {
	//	for (auto it = item.begin(); it != item.end(); ++it)
	//		it.value()->setVisible(false);
 //   }
	//clearItems();
	//m_bars.clear();
	//m_posTextLabel.clear();
 //   this->replot();
	// 1. 清空所有图表 (Bars, Graphs, Curves...)
	clearPlottables();

	// 2. 清空所有辅助项 (Text, Lines, Arrows...)
	clearItems();

	// 3. 重绘
	replot();
	m_bars.clear();
	m_posTextLabel.clear();
}

QCPGraph *CustomChart::creatGraph(QString name)
{
    QCPGraph *graph = this->addGraph();
    graph->setName(name);
    // 2. 获取分配的颜色
    QColor color = getNextColor();
    // 3. 设置正常状态下的样式
    QPen pen;
    pen.setColor(color);
    pen.setWidth(2); // 正常宽度
    graph->setPen(pen);
    // 4. 设置选中状态下的样式 (关键点：Selection Decorator)
    // QCustomPlot 提供了 Decorator 机制，当 Graph 被选中时自动应用此样式
    QCPSelectionDecorator *decorator = graph->selectionDecorator();
    QPen selectedPen;
    selectedPen.setColor(color); // 选中时保持颜色一致
    selectedPen.setWidth(5);     // 选中时线条变粗
    decorator->setPen(selectedPen);

    // 也可以设置选中时的散点样式（可选）
    // decorator->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 5));
    m_graph[name]=graph;
    return graph;
}

QCPBars *CustomChart::creatBar(QString name)
{
    // 1. 创建柱状图对象 (QCPBars)
    QCPBars *myBars = new QCPBars(xAxis, yAxis);
    myBars->setName(name);
    myBars->setPen(QPen(QColor(0, 168, 140).lighter(130))); // 边缘颜色
    myBars->setBrush(QColor(0, 168, 140)); // 填充颜色
    myBars->setWidth(0.6); // 设置柱子的宽度 (坐标轴单位，1表示两个刻度间填满)
    m_bars[name]= myBars;
    return myBars;
}

void CustomChart::creatPosText(QString name, double xpos, double ypos)
{
    QCPItemText* textLabel;
    if (m_posTextLabel[name].contains(xpos)) {
        textLabel = m_posTextLabel[name][xpos];
    }
    else {
        textLabel = new QCPItemText(this);
		// 1. 设置位置类型为 "PlotCoords" (意味着我们可以使用 X轴和 Y轴的坐标数值)
		textLabel->position->setType(QCPItemPosition::ptPlotCoords);
		textLabel->setPositionAlignment(Qt::AlignBottom | Qt::AlignHCenter);
		textLabel->setFont(QFont("Arial", 10)); // 设置字体大小
		textLabel->setColor(Qt::black);         // 设置文字颜色
		// textLabel->setPadding(QMargins(0, 0, 0, 5)); // 如果觉得离柱子太近，可以加点下边距(padding)
        m_posTextLabel[name][xpos] = textLabel;
    }
	//设置坐标位置 (X = 柱子的位置, Y = 柱子的高度)
	textLabel->position->setCoords(xpos, ypos);
	//设置显示的文本 (可以将数字转为字符串)
	textLabel->setText(QString::number(ypos));
    textLabel->setVisible(ypos != 0);
}

void CustomChart::setChartTheme()
{
    xAxis->setBasePen(QPen(Qt::white, 1));
    yAxis->setBasePen(QPen(Qt::white, 1));
    xAxis->setTickPen(QPen(Qt::white, 1));
   yAxis->setTickPen(QPen(Qt::white, 1));
   xAxis->setSubTickPen(QPen(Qt::white, 1));
   yAxis->setSubTickPen(QPen(Qt::white, 1));
   xAxis->setTickLabelColor(Qt::white);
   yAxis->setTickLabelColor(Qt::white);
   xAxis->grid()->setPen(QPen(QColor(140, 140, 140), 1, Qt::DotLine));
   yAxis->grid()->setPen(QPen(QColor(140, 140, 140), 1, Qt::DotLine));
   xAxis->grid()->setSubGridPen(QPen(QColor(80, 80, 80), 1, Qt::DotLine));
   yAxis->grid()->setSubGridPen(QPen(QColor(80, 80, 80), 1, Qt::DotLine));
   xAxis->grid()->setSubGridVisible(true);
   yAxis->grid()->setSubGridVisible(true);
   xAxis->grid()->setZeroLinePen(Qt::NoPen);
   yAxis->grid()->setZeroLinePen(Qt::NoPen);
   xAxis->setUpperEnding(QCPLineEnding::esSpikeArrow);   //坐标系箭头
   yAxis->setUpperEnding(QCPLineEnding::esSpikeArrow);
    QLinearGradient plotGradient;
      plotGradient.setStart(0, 0);
      plotGradient.setFinalStop(0, 350);
      plotGradient.setColorAt(0, QColor(80, 80, 80));
      plotGradient.setColorAt(1, QColor(50, 50, 50));
      setBackground(plotGradient);
      QLinearGradient axisRectGradient;
      axisRectGradient.setStart(0, 0);
      axisRectGradient.setFinalStop(0, 350);
      axisRectGradient.setColorAt(0, QColor(80, 80, 80));
      axisRectGradient.setColorAt(1, QColor(30, 30, 30));
      axisRect()->setBackground(axisRectGradient);
      legend->setMargins(QMargins(3, 2, 2, 2));
      legend->setVisible(true);
      legend->setRowSpacing(0);
      legend->setColumnSpacing(0);
      legend->setBrush(QBrush(Qt::transparent));     //设置图例透明无边框
      legend->setBorderPen(Qt::NoPen);
      legend->setTextColor(Qt::white);
      xAxis->setLabelColor(QColor(255, 255, 255));
      yAxis->setLabelColor(QColor(255, 255, 255));
}

void CustomChart::mouseDoubleClickEvent(QMouseEvent * event)
{
    QCustomPlot::mouseDoubleClickEvent(event);
    if (!m_interactions)return;
    xAxis->setRange(m_axisRangeX.x(), m_axisRangeX.y());
    yAxis->setRange(m_axisRangeY.x(), m_axisRangeY.y());
    //rescaleAxes();
    replot();
}

