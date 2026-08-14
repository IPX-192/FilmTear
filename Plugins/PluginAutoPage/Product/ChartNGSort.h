#ifndef CHARTNGSORT_H
#define CHARTNGSORT_H

#include <QWidget>
#include "ProductDef.h"

namespace Ui {
class ChartNGSort;
}
class CustomChart;
class ChartNGSort : public QWidget
{
    Q_OBJECT

public:
    explicit ChartNGSort(QWidget *parent = nullptr);
    ~ChartNGSort();

public:
    void  UpdateChart(StationInfo* info);

protected:
    CustomChart* m_chart=nullptr;
	int   m_barsShow = 5;

private:
    Ui::ChartNGSort *ui;
};

#endif // CHARTNGSORT_H
