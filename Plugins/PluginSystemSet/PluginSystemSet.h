#ifndef PLUGINSYSTEMSET_H
#define PLUGINSYSTEMSET_H

#include "VISFramePluginModel.h"
#include "MesHttpPost.h"

class PluginSystemSet:public VISFramePluginModel
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID Plugin_Interface_iid FILE "QPlugin.json")
    Q_INTERFACES(Plugin_Interface)
public:
    PluginSystemSet();

public:
    virtual void InitWidgetList(Plugin_Interface* plugin);
    virtual void InitActionList(Plugin_Interface* plugin);
    virtual int OnInitialized();

protected:

public slots:
    int event_ShowLogin();
    //MES接口总线
    int event_MesValidateNumber(QString sn, bool& outValidate);                    //条码校验(PCB扫码后)
    int event_MesSaveProcessOpResult(QString sn, int opResult,
                                     QList<DataDetail> detailAll, QString& outMainId); //保存工序操作结果
    int event_MesCompleteTask(QString sn, bool isSuccess, QString errCode,
                              QString errInfo, bool bindMat, bool& outTaskResult); //工序过站

};

#endif // PLUGINMANUALDEBUG_H
