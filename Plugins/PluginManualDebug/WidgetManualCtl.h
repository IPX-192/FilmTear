#ifndef WIDGETMANUALCTL_H
#define WIDGETMANUALCTL_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QComboBox>
#include <QPushButton>
#include <QButtonGroup>
#include <QRadioButton>
#include "ParamManager.h"
#include "VisUIParam.h"
#include "ParamDef.h"

#pragma execution_character_set("utf-8")

class WidgetManualCtl : public QWidget
{
    Q_OBJECT
public:
    explicit WidgetManualCtl(QWidget *parent = nullptr);

private:
    QGroupBox* CreateMaterialGroup(const QString& groupName, const QString& comboObjName, TrayFunc funcType);
};

#endif // WIDGETMANUALCTL_H
