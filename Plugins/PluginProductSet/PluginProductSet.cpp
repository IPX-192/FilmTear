#include "PluginProductSet.h"
#include "WidgetProductSet.h"
#include "VisAppBus.h"
#include "ParamManager.h"

PluginProductSet::PluginProductSet()
{
	pluginID = "PluginManualDebug";
	pluginVersion = "1.0.1";
	pluginAuther = "wangwei";
	pluginAuthority = OPERATOR;
    loadOrder = 1;
    showOrder = 3;
}

void PluginProductSet::InitSubscibeEvent(Plugin_Interface* plugin)
{
	
}

void PluginProductSet::InitWidgetList(Plugin_Interface *plugin)
{
    WidgetProductSet* widget=new WidgetProductSet();
    plugin->page=widget;
    plugin->iconArea = QSize(40, 40);
    plugin->iconSize = 25;
    plugin->btnHeight = 45;
    plugin->icons << 0xf036 << 0xf249 << 0xf055 << 0xf036 << 0xf249 << 0xf055 << 0xf036 << 0xf055;
}

void PluginProductSet::InitActionList(Plugin_Interface *plugin)
{

}

int PluginProductSet::OnInitialized()
{
    bool bRet = GlobalParam->LoadRecipeProduct();
    ShowSystemLog(bRet ? Log_Info : Log_Error, QString(u8"产品型号文件加载%1！").arg(bRet ? u8"成功" : u8"失败"));
	WidgetProductSet* widget = (WidgetProductSet*)this->page;
	widget->LoadUIParam();

    tagOutputInfo outInfo;
    outInfo._type = INFT_ProductChange;
    emit GlobalParam->frameCore->sig_OutputInfo(outInfo);
	return 0;
}



