#include "WidgetBox.h"
#include "ui_WidgetBox.h"
#include "VisUIParam.h"
#include "ParamManager.h"


WidgetBox::WidgetBox(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WidgetBox)
{
    ui->setupUi(this);
}

WidgetBox::~WidgetBox()
{
    delete ui;
}

void WidgetBox::LoadUIParam()
{
    QString filename = GlobalParam->hardwareParam.fileName;
    VisUIParam::LoadIniToUI(filename, this, &GlobalParam->hardwareParam);
}

void WidgetBox::SaveUIParam()
{
    QString filename = GlobalParam->hardwareParam.fileName;
    VisUIParam::SaveUIToIni(filename, this, &GlobalParam->hardwareParam);
}

void WidgetBox::UpdateParamToUI()
{
    VisUIParam::UpdateParamToUI(&GlobalParam->hardwareParam, this);
}
