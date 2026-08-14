#include "PluginAutoPage.h"
#include <functional>
#include "WidgetAutoPage.h"
#include "WidgetStatusBar.h"
#include "VisAppBus.h"
#include <QMessageBox>
#include "ParamManager.h"

PluginAutoPage::PluginAutoPage()
{
    pluginID = "PluginAutoPage";
    pluginVersion = "1.0.1";
	pluginAuther = "wangwei";
    pluginAuthority = OPERATOR;
    showOrder = 1;
}

void PluginAutoPage::InitWidgetList(Plugin_Interface *plugin)
{
    WidgetAutoPage* widget=new WidgetAutoPage();
    widget->InitLog(frameCore->listPluginLog);
    plugin->page=widget;
    plugin->icons << 0xf080 << 0xf03e << 0xf1fe << 0xf133 << 0xf133 << 0xf133<< 0xf1fe << 0xf133 << 0xf133 << 0xf133;
    plugin->iconArea = QSize(40, 40);
    plugin->iconSize = 25;
    plugin->btnHeight = 45;

    WidgetStatusBar*widgetStatus=new WidgetStatusBar();
    PluginWidgetInfo* pluginWidget = new PluginWidgetInfo();
    pluginWidget->_widget = widgetStatus;
    pluginWidget->_widgetDetail = u8"状态栏";
    plugin->listWidget.push_back(pluginWidget);
}

void PluginAutoPage::InitActionList(Plugin_Interface *plugin)
{
    PluginActionInfo* action1 = new PluginActionInfo();
    action1->_actionName = "ShowConsumables";
    action1->_actionDetail = tr("易损件查看");
    action1->_pAction = (FPTR_ACTION)(&PluginAutoPage::ShowConsumables);
    plugin->listAction.append(action1);
}

void PluginAutoPage::ShowConsumables(bool checkState)
{
	if (GlobalParam->frameCore->curUserInfo.authority == OPERATOR) {
		QMessageBox::warning(nullptr, QString(u8"警告"), QString(u8"当前用户无权限"));
		return;
	}
    VisAppBus::sendEvent("ShowConsumables");
}

