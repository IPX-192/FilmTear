#ifndef PLUGINPRODUCTSET_H
#define PLUGINPRODUCTSET_H

#include "VISFramePluginModel.h"

class PluginProductSet:public VISFramePluginModel
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID Plugin_Interface_iid FILE "QPlugin.json")
    Q_INTERFACES(Plugin_Interface)
public:
    PluginProductSet();

public:
    virtual void InitSubscibeEvent(Plugin_Interface* plugin);
    virtual void InitWidgetList(Plugin_Interface* plugin);
    virtual void InitActionList(Plugin_Interface* plugin);
    virtual int  OnInitialized();
};

#endif // PLUGINMANUALDEBUG_H
