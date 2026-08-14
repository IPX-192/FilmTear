#ifndef THREADBOX_H
#define THREADBOX_H

#include <QObject>
#include "hthread.h"
#include "ThreadGrabDef.h"

enum TrayOperate {
	InTray,   //料仓向搬运模块供料
	OutTray,  //搬运模块向料仓退料
};

struct FeedBoxParam {
	QString  boxName;              //料箱名
	int      layers = 5;           // 料仓层数
	QString  nameAxisZ;            //料仓运动轴名
	QString  nameAxisTrayY;        //料盘运动轴名

	QString  trayWaitPos;          //取料电机取料盘等待位
	QMap<TrayOperate, double> trayPosIn_Out;  //取料电机料盘进出盘点位名
	QVector<double>vecCheckTrayExistZ;      //检查料仓托盘存在Z
	QVector<double>vecBoxUpPosZ;    //取料电机进料仓,料仓Z
	QVector<double>vecFixTrayPosZ;  //托盘固定到取料电机,料仓Z
	QVector<double>vecMotorBackZ;   //取料电机退出料仓Z

	QString      checkBoxExist;           //检查料仓存在
	QString      checkBoxTrayExist;       //检查料仓料盘or模组存在
	QStringList      checkTray;              //检查取料电机料盘存在
	double       layerPitchZ = 0;         //层间Z间距（每层Z偏移量）
};

class ThreadBox: public QObject, public HThread
{
    Q_OBJECT
public:
    explicit ThreadBox(TrayFunc type,QObject *parent = nullptr);
	~ThreadBox();

public:
	void InitParam();
	

protected:
	virtual bool doTask();
	int  ProcessTray(std::pair<int, TrayOperate> trayTask);
	int  CheckFeedBox(bool& existTray, int& layer);
	bool QueryInPos(QString posname);
	void UpdateBoxParam();

protected:
	QString   m_errInfo;
    const TrayFunc m_funcType;
	FeedBoxParam   m_boxParam;     //料箱参数
	QString  m_logType;
	std::mutex              m_mutex;
	std::condition_variable m_condition;
	bool                    m_flagTaskInTray = false;   //进盘请求
	QList<int>              m_listTask;               //退料盘任务队列layer
	int                     m_manualLayer = -1;       //手动指定层(-1=自动扫描)
	int                     m_curLayer = 0;           //当前处理的层数
	bool                    m_flagNoMaterial=true;              //料仓无料标志

protected:
	void sigBoxTrayStatus(TrayFunc type,int layer,QString info);
	void sigTrayReady(TrayFunc type);

protected slots:
	int   event_InTrayTask(TrayFunc type);     //请求进盘任务处理
	int   event_BlankTray(TrayFunc type);
	int   event_SupplyMaterial();
    //手动调试界面触发拉料和退料
    int   event_InTrayTaskLayer(TrayFunc type,int layer);
    int   event_BlankTrayLayer(TrayFunc type,int layer);
};

#endif // THREADBOX_H
