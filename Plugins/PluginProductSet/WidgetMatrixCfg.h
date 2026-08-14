#ifndef WIDGETMATRIXCFG_H
#define WIDGETMATRIXCFG_H

#include <QWidget>
#include <QStandardItemModel>

namespace Ui {
class WidgetMatrixCfg;
}

class WidgetMatrixCfg : public QWidget
{
    Q_OBJECT

public:
    explicit WidgetMatrixCfg(QWidget *parent = nullptr);
    ~WidgetMatrixCfg();

protected:
    void  InitTable();

protected:
   QStandardItemModel*  m_pModel;
   QVector<QStringList> m_vecMatrix;

private slots:
   void on_btnNewMatrix_clicked();

   void on_btnDeleteMatrix_clicked();

public slots:
	void  LoadUIParam();
	void  SaveUIParam();
	void  UpdateParamToUI();

signals:
    void  sigAddPlatfrom();

private:
    Ui::WidgetMatrixCfg *ui;
};

#endif // WIDGETMATRIXCFG_H
