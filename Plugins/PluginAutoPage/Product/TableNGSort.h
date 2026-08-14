#ifndef TABLENGSORT_H
#define TABLENGSORT_H

#include <QWidget>
#include <QStandardItemModel>
#include "ProductDef.h"

namespace Ui {
class TableNGSort;
}

class TableNGSort : public QWidget
{
    Q_OBJECT

public:
    explicit TableNGSort(QWidget *parent = nullptr);
    ~TableNGSort();

public:
    void  UpdateTable(StationInfo* info);

protected:
    void  InitTable();

protected:
    QStandardItemModel* m_pModel;
    int    m_rowsNG = 5;

private:
    Ui::TableNGSort *ui;
};

#endif // TABLENGSORT_H
