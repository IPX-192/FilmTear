#include "WidgetGrip.h"
#include "ui_WidgetGrip.h"
#include "VisUIParam.h"
#include "ParamManager.h"


WidgetGrip::WidgetGrip(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WidgetGrip)
{
    ui->setupUi(this);
}

WidgetGrip::~WidgetGrip()
{
    delete ui;
}

void WidgetGrip::LoadUIParam()
{
    QString filename = GlobalParam->hardwareParam.fileName;
    VisUIParam::LoadIniToUI(filename, this, &GlobalParam->hardwareParam);
}

void WidgetGrip::SaveUIParam()
{
    QString filename = GlobalParam->hardwareParam.fileName;
    VisUIParam::SaveUIToIni(filename, this, &GlobalParam->hardwareParam);
}

void WidgetGrip::UpdateParamToUI()
{
    VisUIParam::UpdateParamToUI(&GlobalParam->hardwareParam, this);
}
