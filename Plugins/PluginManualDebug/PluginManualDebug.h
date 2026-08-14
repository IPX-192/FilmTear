#ifndef PLUGINMANUALDEBUG_H
#define PLUGINMANUALDEBUG_H

#include "VISFramePluginModel.h"

class  PluginManualDebug:public VISFramePluginModel
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID Plugin_Interface_iid FILE "QPlugin.json")
    Q_INTERFACES(Plugin_Interface)
public:
    PluginManualDebug();

public:
    virtual void InitSubscibeEvent(Plugin_Interface* plugin);
    virtual void InitWidgetList(Plugin_Interface* plugin);
    virtual void InitActionList(Plugin_Interface* plugin);
    virtual int  OnInitialized();

protected:
    void  ShowMotorCtr(bool checkState);
};

#endif // PLUGINMANUALDEBUG_H
