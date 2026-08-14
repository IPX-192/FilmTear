#ifndef THREADGRABDEF_H
#define THREADGRABDEF_H

#include <QObject>
#include "ParamManager.h"



enum TransStatus {

	StatusFeedGrab,     //上料抓取
	//StatusTrayUse,      //托盘使用
	StatusFeedPlace,    //上料放置
	StatusBlankGrab,    //下料抓取
	StatusBlankPlace,   //下料放置
};

//夹爪定义
struct GripDef {
	int index = 0;
	QString name;           //夹爪名
	QString axisName;       //轴名
	bool  useFlag = true;   //是否使用
	QString safeZ;          //安全位
	int grabDelay = 500;    //没有张开和闭合检测时靠延时确认
	int timeout = 5000;     //等待夹爪超时时间
};

//托盘参数
struct TrayPosParam
{
    int rows = 2;
    int colomns = 2;
    QVector<QVector4D> xyPos;   //x:电机夹爪，y:上下料盘电机
	TrayPosParam(){
		xyPos.resize(4);
	}
};


////模组信息
//struct ModuleInfo {
//	bool result = true;             //测试结果
//    QString  trayBarCode;           //托盘条码
//	bool qrcodeflag = true;         //扫码结果
//	int  indexTray;                 //上料托盘序号
//	int  station = 0;               //测试工位/穴位
//	int  indexGripFeed = -1;         //上料夹爪序号,备用
//	int  indexGripBlank = -1;        //下料夹爪序号,备用
//	int  totalPlace = 0;            //单批上料夹爪放置模组总个数
//	QString qrcode;                 //二维码
//	QString ngReason = "";          //NG原因
//};
//Q_DECLARE_METATYPE(ModuleInfo)



#endif // THREADGRABDEF_H
