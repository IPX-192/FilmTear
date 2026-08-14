#ifndef CHARTPRODUCEHOUR_H
#define CHARTPRODUCEHOUR_H

#include <QWidget>
#include "ProductDef.h"

namespace Ui {
class ChartProduceHour;
}
class CustomChart;
class ChartProduceHour : public QWidget
{
    Q_OBJECT

public:
    explicit ChartProduceHour(QWidget *parent = nullptr);
    ~ChartProduceHour();

public:
    void UpdateChart(StationInfo* info);

protected:
	CustomChart* m_chart = nullptr;
	int    m_hoursShow = 12;

private:
    Ui::ChartProduceHour *ui;
};

#endif // CHARTPRODUCEHOUR_H
