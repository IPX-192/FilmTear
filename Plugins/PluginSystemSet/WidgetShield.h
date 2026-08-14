#ifndef SHIELD_PAGE_H
#define SHIELD_PAGE_H

#include <QWidget>
#include <QAbstractTableModel>
#include <QTableView>
#include <QLineEdit>
#include <QCheckBox>
#include <QVector>
#include <QPushButton>
#include "ParamManager.h"

struct ShieldInfoPtr {
	QString name;
	bool* checked;
	QString pointAddress;
};

class ShieldTableModel;
class WidgetShield : public QWidget
{
    Q_OBJECT

public:
    explicit WidgetShield(QWidget *parent = nullptr, const QString& iniFilePath = QString());
    ~WidgetShield();

public slots:
    void LoadUIParam();
    void SaveUIParam();
    void UpdateParamToUI();

private:
    void InitializeUI();
    void InitializeData();
    void InsertRow(const QString &name, bool checked, const QString &address);
    
    // Member variables
    ShieldTableModel *m_model;
    QTableView *m_tableView;
    QString m_iniFilePath;  //
	QMap<QString, QPair<bool*, QString>>m_shieldDataMap; //名称-（是否使用，点位地址）
};

#endif // SHIELD_PAGE_H
