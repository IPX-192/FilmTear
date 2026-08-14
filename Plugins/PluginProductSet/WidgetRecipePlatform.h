#ifndef WIDGETRECIPEPLATFORM_H
#define WIDGETRECIPEPLATFORM_H

#include <QWidget>
#include <QTableView>
#include <QStandardItemModel>

namespace Ui {
class WidgetRecipePlatform;
}

class WidgetRecipePlatform : public QWidget
{
    Q_OBJECT

public:
    explicit WidgetRecipePlatform(QWidget *parent = nullptr);
    ~WidgetRecipePlatform();

protected:
    void  InitTable();

public slots:
	void  LoadUIParam();
	void  SaveUIParam();
    void  UpdateParamToUI();

public slots:
    void  slotAddPlatfrom();

protected:
   QStandardItemModel*  m_pModel;

private:
    Ui::WidgetRecipePlatform *ui;
};

#endif // WIDGETRECIPEPLATFORM_H
