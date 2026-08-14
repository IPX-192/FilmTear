#include "WidgetStationProduct.h"
#include "ui_WidgetStationProduct.h"

WidgetStationProduct::WidgetStationProduct(StationInfo* stationInfo, QWidget *parent) :
    m_stationInfo(stationInfo),
    QWidget(parent),
    ui(new Ui::WidgetStationProduct)
{
    ui->setupUi(this);
    connect(ui->tableProduct, &TableProduct::sigClear, [=](bool all) {
		emit sigClearData(m_stationInfo->station,all);
	});
       
}

WidgetStationProduct::~WidgetStationProduct()
{
    delete ui;
}

void WidgetStationProduct::UpdateWidget()
{
    ui->tableProduct->UpdateTable(m_stationInfo);
    ui->tableNGSort->UpdateTable(m_stationInfo);
    ui->produceHour->UpdateChart(m_stationInfo);
    ui->chartNGSort->UpdateChart(m_stationInfo);
}
