#include "PluginManualDebug.h"
#include "WidgetManualDebug.h"
#include <QMessageBox>
#include "VisAppBus.h"
#include "ParamManager.h"
#include "WidgetManualDebug.h"
#include "WidgetMotorCtrl.h"
#include "VisUIParam.h"

PluginManualDebug::PluginManualDebug()
{
	pluginID = "PluginManualDebug";
	pluginVersion = "1.0.1";
	pluginAuther = "wangwei";
	pluginAuthority = OPERATOR;
    showOrder = 2;
}

void PluginManualDebug::InitSubscibeEvent(Plugin_Interface* plugin)
{
	
}

void PluginManualDebug::InitWidgetList(Plugin_Interface *plugin)
{
    WidgetManualDebug* widget = new WidgetManualDebug();
    plugin->page=widget;
    plugin->icons << 0xf080 << 0xf03e << 0xf1fe << 0xf133<< 0xf03e<< 0xf133;
    plugin->iconArea = QSize(40, 40);
    plugin->iconSize = 25;
    plugin->btnHeight = 45;

	PluginLogInfo pluginLog;
	pluginLog.type = DebugLog;
	pluginLog.index = 0;
	pluginLog._pLog = std::bind(&WidgetManualDebug::AddLog, widget, std::placeholders::_1, std::placeholders::_2);
	frameCore->listPluginLog.append(pluginLog);
}

void PluginManualDebug::InitActionList(Plugin_Interface *plugin)
{
	PluginActionInfo* action1 = new PluginActionInfo();
	action1->_actionName = "ShowMotorCtr";
	action1->_actionDetail = tr("电机控制");
	action1->_pAction = (FPTR_ACTION)(&PluginManualDebug::ShowMotorCtr);
	plugin->listAction.append(action1);
}

int PluginManualDebug::OnInitialized()
{
	WidgetMotorCtrl::instance()->InitMotor();
	WidgetManualDebug* widget =  (WidgetManualDebug*)page;
	widget->LoadUIParam();
	return 0;
}

void PluginManualDebug::ShowMotorCtr(bool checkState)
{
	if (GlobalParam->frameCore->curUserInfo.authority == OPERATOR) {
		QMessageBox::warning(nullptr, QString(u8"警告"), QString(u8"当前用户无权限"));
		return;
	}
	WidgetMotorCtrl::instance()->show();
}



