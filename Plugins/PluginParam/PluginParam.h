#ifndef PLUGINPARAM_H
#define PLUGINPARAM_H

#include "VISFramePluginModel.h"

class PluginParam:public VISFramePluginModel
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID Plugin_Interface_iid FILE "QPlugin.json")
    Q_INTERFACES(Plugin_Interface)
public:
    PluginParam();

public:
    virtual bool ConnectCore(QObject* core);
    virtual void InitActionList(Plugin_Interface* plugin);
    virtual int  OnInitialized();

protected:
    void  ShowSwitchUser(bool checkState);
};

#endif // PLUGINPARAM_H
