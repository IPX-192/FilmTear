#ifndef ProductDetailForm_H
#define ProductDetailForm_H

#include <QWidget>
#include <QStandardItemModel>

namespace Ui {
class ProductDetailForm;
}

struct ItemDetail
{
    QString name = "NULL";
    double maxValue = 0;
    double minValue = 0;
    QString sUnit = "NULL";
    double testValue = 0;
    bool  result=false;
};

class ProductDetailForm : public QWidget
{
    Q_OBJECT

public:
    explicit ProductDetailForm(int station,QWidget *parent = nullptr);
    ~ProductDetailForm();

    void  InitData(QString barcode);
    void  AddData(ItemDetail&info);

protected:
    void  InitTable();

protected:
    QStandardItemModel* m_pModel;
    int   m_station=0;

private:
    Ui::ProductDetailForm *ui;
};

#endif // ProductDetailForm_H
