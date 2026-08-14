#ifndef CYLINDERCTRL_H
#define CYLINDERCTRL_H

#include <QObject>
#include "Singleton.h"
#include "ParamDef.h"

class CylinderCtrl : public QObject
{
    Q_OBJECT
    SINGLETON_DECL(CylinderCtrl)
public:
    explicit CylinderCtrl(QObject *parent = nullptr);

    struct IOState{
        QString ioName;
        int level;
    };

public:
    int  CheckIoState(QString ioName,int level,QString errInfo);
    int  CheckIoState(QStringList ioName, int level, QString errInfo);
    int  SetDoubleIOOut(IOState out,IOState in1,IOState in2,QString errInfo);
    int  SetSigIOOut(IOState out,IOState in1, IOState in2,QString errInfo);
	int  SetSigIOOut(IOState out, IOState in, QString errInfo);
	int  SetDoubleIOOut(IOState out, IOState in, QString errInfo);
signals:

public slots:
    int   event_SetBlockUp(TrayFunc type, bool up);    //µ²¸×
    int   event_SetPushUp(TrayFunc type, bool up);    //¶¥Éý
    int   event_CheckSkew(TrayFunc type);             //ÍáÐ±
    int   event_SetGripClose1(TrayFunc type, bool close);  //¼Ð×¦¼Ð½ô£¬´øCheck
    int   event_SetCleanPCBGripUp(int index, bool up);    //0:Á÷ÏßÇåÏ´¼Ð×¦  1:×ªÅÌÇåÏ´¼Ð×¦
    int   event_SetTurntableGripUp(int index, bool up);   //×ªÅÌÉÏÏÂÁÏ¼Ð×¦
	
};

#endif // CYLINDERCTRL_H
