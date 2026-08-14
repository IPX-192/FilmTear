#include "ChartProduceHour.h"
#include "ui_ChartProduceHour.h"
#include "Chart/CustomChart.h"

int kk = 1;
ChartProduceHour::ChartProduceHour(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ChartProduceHour)
{
    ui->setupUi(this);
    m_chart =new CustomChart(this);
    layout()->addWidget(m_chart);
    m_chart->setChartTitle(u8"时段产出");
    m_chart->setAxisXRange(-0.5, m_hoursShow);
    m_chart->setAxisYRange(0, 300);
}

ChartProduceHour::~ChartProduceHour()
{
  
    delete ui;
}

void ChartProduceHour::UpdateChart(StationInfo* info)
{
	QVector<QString>  listLabel;
	QVector<double>xData, yData;
    QVector<int> targetKey;
	int currentHour = QTime::currentTime().hour();
    for (int i = m_hoursShow - 1; i >= 0; --i) {
        int targetHour = (currentHour - i + 24) % 24;
        int output = info->numHour.value(targetHour, 0);
        // key 使用简单的 0, 1, 2... 11 索引即可
        targetKey << targetHour;
        xData << (m_hoursShow - 1 - i); // 这样第0个柱子对应最旧的时间
        yData << output;
        // 生成 X 轴标签，例如 "14:00"
        listLabel << QString("%1:00").arg(targetHour);
    }
    //其他时间段置0
    for (auto it = info->numHour.begin(); it != info->numHour.end(); ++it) {
        if (!targetKey.contains(it.key()) ){
            it.value() = 0;
        }
    }
	m_chart->addBars(u8"时段", listLabel, xData, yData);
}
