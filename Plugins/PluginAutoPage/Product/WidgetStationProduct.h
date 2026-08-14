#ifndef WIDGETSTATIONPRODUCT_H
#define WIDGETSTATIONPRODUCT_H

#include <QWidget>
#include <QMap>
#include "ProductDef.h"

namespace Ui {
class WidgetStationProduct;
}

class WidgetStationProduct : public QWidget
{
    Q_OBJECT

public:
    explicit WidgetStationProduct(StationInfo*stationInfo,QWidget *parent = nullptr);
    ~WidgetStationProduct();

public:
	void  UpdateWidget();

protected:
    int   m_station = 0;
	StationInfo* m_stationInfo = nullptr;

signals:
    void  sigClearData(int station,bool all);

private:
    Ui::WidgetStationProduct *ui;
};

#endif // WIDGETSTATIONPRODUCT_H
