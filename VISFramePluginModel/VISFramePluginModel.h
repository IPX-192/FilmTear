#ifndef VISFRAMEPLUGINMODEL_H
#define VISFRAMEPLUGINMODEL_H

#include <QtCore/qglobal.h>
#include "../interface/coreinterface.h"

#if defined(VISFRAMEPLUGINMODEL_LIBRARY)
#  define VISFRAMEPLUGINMODEL_EXPORT Q_DECL_EXPORT
#else
#  define VISFRAMEPLUGINMODEL_EXPORT Q_DECL_IMPORT
#endif

class VISFRAMEPLUGINMODEL_EXPORT VISFramePluginModel:public Plugin_Interface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID Plugin_Interface_iid FILE "QPlugin.json")
    Q_INTERFACES(Plugin_Interface)
public:
    VISFramePluginModel();
    ~VISFramePluginModel();

    //连接Core，使Plugin获得VISFrameCore对象实例
    bool  ConnectCore(QObject* core);

public slots:
    void  slotAction(bool checkState);
};

#endif // VISFRAMEPLUGINMODEL_H
