#ifndef PLUGINMEASUREPROCESS_H
#define PLUGINMEASUREPROCESS_H

#include "VISFramePluginModel.h"
#include "VisMotorManager.h"
#include "ParamDef.h"

class ThreadFeed;
class ThreadClean;
class TurntableGrab;
class PipeLineManager;
class PluginMeasureProcess:public VISFramePluginModel
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID Plugin_Interface_iid FILE "QPlugin.json")
    Q_INTERFACES(Plugin_Interface)
public:
    PluginMeasureProcess();
	~PluginMeasureProcess();

	void InitActionList(Plugin_Interface* plugin);
	int  OnInitialized();


protected:
	void sigMachineStatus(MachineRunStatus status);

public slots:
	int  event_InitPosParam();
    //清料
    int  event_ClearModule();
	//自动流程启动
	int  event_AutoStart();
	//暂停
	int  event_AutoPause();
	//恢复
	int  event_AutoResume();
	//急停
	int  event_AutoEmg();
	//复位
	int  event_AutoReset();
	//清除报警
	int  event_ClearWarning();
	//弹窗提示报警
	int  event_PopupWarning(QString info);
	//弹窗提示报警
	int  event_PopupErrNotify(QString info, QStringList iocheck, QVector<int> normalLevel);
	//弹窗报警
	int  event_PopupErrInfo(QString info);
    //弹窗提示
    int  event_PopupUserMsgBox(QStringList listBtn,QString info);
    int  event_SetProductEnd(bool end);
    int  event_GetProductEnd(bool& end);
	int  event_TestDirtyDetect(int station);

protected:
	void InitSlot();
	int  ResetPreCheck();              //复位前检查
	int  CheckIO();                    //检查模组是否清料完毕,料盘等
	int  InitIO();                     //初始化IO状态
	int  CheckDevicePos();             //检查电机当前位置，到初始点位是否会撞机
	int  InitDevicePos();              //运动电机到初始点位
	void WaitTime(int ms);
    int  DoClearModule();
	 

private slots:
    //关联信号
    void  SlotIoChange(QMap<QString,int> inStateMap);

protected:
	bool  m_inReset = false; //复位中标志
    bool  m_clearModule=false;
    bool  m_isEmgFlag = false;
    bool  m_manualFeeding = false;
    bool  m_waitTray = false;      //待料状态
    bool  m_productEnd = false;      //结束生产标志
    VisMotorToolSpace::IOLevel  m_ioState = VisMotorToolSpace::IO_ON;

    ThreadFeed*m_threadFeed[2]={nullptr,nullptr};
    ThreadClean*m_threadClean=nullptr;
    TurntableGrab*m_turntableGrab=nullptr;
    PipeLineManager* m_pipeLineManager = nullptr; //流水线管理器
};

#endif // PLUGINMEASUREPROCESS_H
