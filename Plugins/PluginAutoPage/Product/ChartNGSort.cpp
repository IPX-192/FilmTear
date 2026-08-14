#include "ChartNGSort.h"
#include "ui_ChartNGSort.h"
#include "Chart/CustomChart.h"

ChartNGSort::ChartNGSort(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ChartNGSort)
{
    ui->setupUi(this);
	m_chart = new CustomChart(this);
	layout()->addWidget(m_chart);
	m_chart->setChartTitle(u8"不良信息");
	m_chart->setAxisXRange(-0.5, m_barsShow - 0.5);
}

ChartNGSort::~ChartNGSort()
{
    delete ui;
}

void ChartNGSort::UpdateChart(StationInfo* info)
{
	QList<QPair<QString, int>> list;
	for (auto it = info->ngInfo.constBegin(); it != info->ngInfo.constEnd(); ++it) {
		list.append(qMakePair(it.key(), it.value()));
	}
	// 3. 使用 std::sort 自定义排序规则
	// 这里的 lambda 表达式决定了排序逻辑
	std::sort(list.begin(), list.end(), [](const QPair<QString, int>& a, const QPair<QString, int>& b) {
		// return a.second < b.second; // 升序 (从小到大)
		return a.second > b.second; // 降序 (从大到小)
		});
	QVector<QString>  listLabel;
	QVector<double> xData,yData;
	for (int i = 0; i < m_barsShow;i++) {
		xData << xData.size();
		if (i >= list.size()) {
			listLabel << "";
			yData << 0;
		}
		else {
			listLabel << list.at(i).first;
			yData << list.at(i).second;
		}
	}
	m_chart->addBars(u8"排序", listLabel, xData, yData);	
}
