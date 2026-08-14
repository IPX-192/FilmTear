#include "WidgetLogAll.h"
#include "ui_WidgetLogAll.h"
#include "ParamManager.h"

WidgetLogAll::WidgetLogAll(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WidgetLogAll)
{
    ui->setupUi(this);
}

WidgetLogAll::~WidgetLogAll()
{
    delete ui;
}

void WidgetLogAll::InitLog(QList<PluginLogInfo>& listPluginLog)
{
	QStringList listLogType;
    listLogType <<Log_PipeLineOnline<< Log_PipeLine << Log_GrabPCB;
    listLogType <<Log_GrabHolder<<Log_Clean<<Log_TurntableGrab;
	QVector<WidgetLog*>listLogWidget;
    listLogWidget << ui->widget_PipeLineOnline << ui->widget_PipeLine<<ui->widget_FeedPCB
                  <<ui->widget_FeedHolder<<ui->widget_PCBClean<<ui->widget_TurntableGrab;
	for (int i = 0; i < listLogType.size(); i++) {
		PluginLogInfo pluginLog;
		pluginLog.type = listLogType[i];
		pluginLog.index = 0;
		pluginLog._pLog = std::bind(&WidgetLog::addLog, listLogWidget[i], std::placeholders::_1, std::placeholders::_2);
		listPluginLog.append(pluginLog);
	}
    listLogWidget.clear();
    listLogWidget<<ui->widget_Fixture1<<ui->widget_Fixture2<<ui->widget_Fixture3<<ui->widget_Fixture4;
    for (int i = 0; i < 4; i++) {
        PluginLogInfo pluginLog;
        pluginLog.type = Log_Fixture;
        pluginLog.index = i;
        pluginLog._pLog = std::bind(&WidgetLog::addLog, listLogWidget[i], std::placeholders::_1, std::placeholders::_2);
        listPluginLog.append(pluginLog);
    }
}
