#ifndef WIDGETLOAD_H
#define WIDGETLOAD_H

#include <QWidget>
#include "MesHttpPost.h"

namespace Ui {
class WidgetLoad;
}

class WidgetLoad : public QWidget
{
    Q_OBJECT

public:
    explicit WidgetLoad(QWidget *parent = nullptr);
    ~WidgetLoad();

protected:
    void closeEvent(QCloseEvent *event) override;

public:
    void SetMaterialInfo(QList<MaterialInfo> infos);
    void ShowMsg(QString msg);

private slots:
    void on_btnLoad_clicked();

    void on_materialInfoId_currentTextChanged(const QString &arg1);

signals:
    void sigLoadDone();
    void sigCloseEvent();

private:
    Ui::WidgetLoad *ui;
    QList<MaterialInfo> m_materialInfo;
};

#endif // WIDGETLOAD_H
