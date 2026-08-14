#ifndef WidgetProductTotal_H
#define WidgetProductTotal_H

#include <QWidget>
#include <QMap>
#include "ProductDef.h"

namespace Ui {
class WidgetProductTotal;
}

class WidgetProductTotal : public QWidget
{
    Q_OBJECT

public:
    explicit WidgetProductTotal(int stationNum, QString filepath,QWidget *parent = nullptr);
    ~WidgetProductTotal();

protected:
    void  InitWidget();
	void  CalcuTotal();    //计算汇总数据
    void  LoadUIParam();
    void  SaveUIParam();
    void  ResetHourData();

protected:
    int  m_stationNum=0;
    QMap<int,StationInfo>m_productInfo;
    QString m_filePath;

public slots:
    int  event_UpdateProduct(ProductTask taskItem);
    void slotClearData(int station, bool all);

private:
    Ui::WidgetProductTotal *ui;
};

#endif // WidgetProductTotal_H
