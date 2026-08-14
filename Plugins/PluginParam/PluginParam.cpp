#include "PluginParam.h"
#include <QDir>
#include <QApplication>
#include "ParamManager.h"
#include "VisAppBus.h"
#include "SwitchUser.h"

PluginParam::PluginParam()
{
	pluginID = "PluginParam";
	pluginVersion = "1.0.1";
	pluginAuther = "wangwei";
    pluginAuthority = OPERATOR;
    loadOrder = 0;
    //QFont font("SimSun", 15); // 设置全局默认字体为Arial，大小为15
    //font.setBold(false);
    //qApp->setFont(font); // 应用全局字体设置
}

bool PluginParam::ConnectCore(QObject* core)
{
	VISFramePluginModel::ConnectCore(core);
	GlobalParam->frameCore = frameCore;
	return true;
}

void PluginParam::InitActionList(Plugin_Interface *plugin)
{
    PluginActionInfo* action1 = new PluginActionInfo();
    action1->_actionName = "ShowSwitchUser";
    action1->_actionDetail = tr("切换用户");
    action1->_pAction = (FPTR_ACTION)(&PluginParam::ShowSwitchUser);
    plugin->listAction.append(action1);
}

int PluginParam::OnInitialized()
{
    QSettings settings(frameCore->configDirPath + "system.ini", QSettings::IniFormat);
    bool flagOffline=settings.value("Device/Offline").toInt();
    GlobalParam->flagOffline = flagOffline;
    VisAppBus::sendEvent("LoginUserChange");
    GlobalParam->hardwareParam.fileName = frameCore->configDirPath + "hardwareParam.ini";
    GlobalParam->systemParam.fileName = frameCore->configDirPath + "systemParam.ini";
    GlobalParam->recipeProduct.curProduct = "12324";
    GlobalParam->recipeProduct.productPath = frameCore->configDirPath + "/Product/";
    QDir dir;
    for(int i=0;i< GlobalParam->recipeProduct.vecRecipeNode.size();i++){
        if(i==0)continue;
        IRecipeBase*recipeBase=GlobalParam->recipeProduct.vecRecipeNode[i].second;
        recipeBase->filepath=GlobalParam->recipeProduct.productPath+recipeBase->objectName()+"/";
        dir.mkpath(recipeBase->filepath);
    }

    return 0;
}

void PluginParam::ShowSwitchUser(bool checkState)
{
    static SwitchUser* widgetUser=nullptr;
    if(widgetUser==nullptr){
        widgetUser=new SwitchUser();
        connect(widgetUser,&SwitchUser::sigSwitchUser,[=](QString username, int level){
                UserInfo& curUserInfo = GlobalParam->frameCore->curUserInfo;
                curUserInfo.userName=username;
                curUserInfo.authority = (AuthorityType)level;
                VisAppBus::sendEvent("LoginUserChange");
            });
    }
    widgetUser->ShowWidget();
}

