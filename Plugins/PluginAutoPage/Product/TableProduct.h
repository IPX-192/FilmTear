#ifndef TABLEPRODUCT_H
#define TABLEPRODUCT_H

#include <QWidget>
#include <QStandardItemModel>
#include "ProductDef.h"

namespace Ui {
class TableProduct;
}

class TableProduct : public QWidget
{
    Q_OBJECT

public:
    explicit TableProduct(QWidget *parent = nullptr);
    ~TableProduct();

public:
    void   UpdateTable(StationInfo* info);

protected:
    void   InitTable();

protected:
    QStandardItemModel* m_pModel;

private:
    Ui::TableProduct *ui;

signals:
    void  sigClear(bool all);   //all:所有工位

public slots:
    void  slotClearBtn();
};

#endif // TABLEPRODUCT_H
