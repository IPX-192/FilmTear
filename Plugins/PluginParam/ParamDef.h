#ifndef PARAMDEF_H
#define PARAMDEF_H

#include <QTime>
#include <QRect>
#include <QObject>
#include <QVector4D>
#include <QVariant>
#include <QSettings>
#include <QDataStream>
#include "../Common/FeildDefine.h"
#include "opencv2/opencv.hpp"

#if defined(PLUGINPARAM_LIBRARY)
#  define PARAMMANAGER_EXPORT Q_DECL_EXPORT
#else
#  define PARAMMANAGER_EXPORT Q_DECL_IMPORT
#endif

#define LOCALTEST  false

#define  StationNum 2
#define  TrayNum 2
#define  FeedGripNum 1
#define  BlankGripNum 1
#define  PanoramicDegree 130


#define FeedHolderCam (QString(u8"上料壳体"))
#define FeedPCBCam (QString(u8"上料PCB"))

#define DirtyDetectCam (QString(u8"灰尘检测"))
#define AssembleTopCam (QString(u8"组装俯拍"))
#define AssembleBottomCam (QString(u8"组装仰拍"))

#define ScanHolderCode (QString(u8"壳体扫码"))   //壳体扫码枪
#define ScanPCBCode    (QString(u8"PCB扫码"))    //PCB扫码枪

#define  Log_PipeLineOnline (QString(u8"流水线连线"))
#define  Log_PipeLine (QString(u8"流水线"))
#define  Log_GrabPCB (QString(u8"PCB搬运"))
#define  Log_GrabHolder (QString(u8"壳体搬运"))
#define  Log_Clean (QString(u8"清洗"))
#define  Log_Fixture (QString(u8"治具"))
#define  Log_TurntableGrab (QString(u8"转盘上料"))


enum MachineRunStatus
{
	Homed,   //回零
	AutoRunning,  //生产
	Pause,     //暂停
	EmergencyStop,  //急停
};


#define Property_Var(type, name, value) \
   Q_PROPERTY(type name MEMBER name) \
   type name=value; \

#define PropertyVar(type, name) \
   Q_PROPERTY(type name MEMBER name) \
   type name; \

enum TrayFunc {
	EmptyBuf,            //空盘缓存
	FeedHolder,          //上料壳体
	FeedPCB,             //上料PCB
	PCBClean,            //PCB清洗
	FeedTurntable,       //转盘上料
};

struct TrayInfo {
    QString    trayCode;        //托盘码
	bool   empty = true;        //是否空盘
	bool   result = true;       //OK盘或NG盘           
	QString    holderBarCode = 0;        //壳体条码
	QString    pcbBarCode = 0;           //PCB条码
	QString    lensBarCode = 0;          //镜头条码
	QString    errInfo;                  //错误信息
    bool       productEnd = false;       //是否结束生产
};

// 2. 重载序列化操作符 (写入流)
inline QDataStream& operator<<(QDataStream& out, const TrayInfo& data) {
	// 按顺序写入结构体成员
    out << data.trayCode << data.empty << data.result << data.holderBarCode << data.pcbBarCode << data.lensBarCode << data.errInfo << data.productEnd;
	return out;
}

// 3. 重载反序列化操作符 (从流读出)
inline QDataStream& operator>>(QDataStream& in, TrayInfo& data) {
	// 严格按照写入的顺序读出
    in >> data.trayCode >> data.empty >> data.result >> data.holderBarCode >> data.pcbBarCode >> data.lensBarCode >> data.errInfo >> data.productEnd;
	return in;
}


//MES校验规则
enum MesCheckType
{
    SENSORID,
    BARCODE,
};

//Matrix项
struct  MatrixSetting {
    QString  platformName;     //平台名称(MES)
    QString  productName;      //产品名称
    QString  productShow;      //上位机显示(机种切换)
    QString  productNo;        //装配序列号
    QString  barCodeLimit1;    //主轨道产品固定条码
    QString  barCodeStartPos1; //固定条码起始位置
    QString  barCodeLimit2;    //次轨道固定条码
    QString  barCodeStartPos2; //固定条码起始位置
    QString  productFamily1;   //主轨道托盘类型
    QString  fixtureSn1;       //主轨道治具码
    QString  productFamily2;   //次轨道托盘类型
    QString  fixtureSn2;       //次轨道治具码
    QString  lensProductNo;    //镜头水洗序列号
    QString  lensCodeEnable;   //镜头码防错启用
};


class IRecipeBase : public QObject
{
    Q_OBJECT
public:
    virtual ~IRecipeBase() = default;
    PropertyVar(QString,filepath)   //配方路径
    PropertyVar(QStringList,listRecipe)   //配方列表
    PropertyVar(QString,curRecipe)   //当前配方
};

//型号参数
class PARAMMANAGER_EXPORT RecipeProduct : public IRecipeBase
{
    Q_OBJECT
public:
    QString          productPath = "";             //产品型号路径
    QVector<MatrixSetting> listPlatformMatrix;      //平台列表映射
    QVector<QPair<QString,IRecipeBase*>>    vecRecipeNode;        //所有配方中文名+对应配方实例
    QVector<QVector<QPair<QString,QString>>>    vecRecipeDetail;    //机种+配方配置映射(配方中文名+值)
 	MatrixSetting*   curMatrix=nullptr;
    QString          curProduct = "AM57";       //当前产品型号
    void  UpdateRecipe();
};


//电机点位配方
class PARAMMANAGER_EXPORT RecipeMotor : public IRecipeBase
{
    Q_OBJECT
public:
};

//视觉配方
class PARAMMANAGER_EXPORT RecipeVision : public IRecipeBase
{
    Q_OBJECT
public:
    RecipeVision &operator=(const RecipeVision&);
    //具体视觉参数
};

//托盘配方
class PARAMMANAGER_EXPORT RecipeTray : public IRecipeBase
{
    Q_OBJECT
public:
    RecipeTray &operator=(const RecipeTray&);
    bool  ReadTrayFile(QString filename,QObject* uiObj);
    void  WriteTrayFile(QString filename,QObject* uiObj);

public:

    Property_Var(int,trayRowsHolder,2)
    Property_Var(int, trayColsHolder,2)
    Property_Var(int, trayRowsPCB, 2)
    Property_Var(int, trayColsPCB, 2)

    Property_Var(int, ngTrayRows, 2)
    Property_Var(int, ngTrayCols, 2)

    Property_Var(int, holderLayerRows, 5)
    Property_Var(int, holderLayerCols, 1)
    Property_Var(int, pcBLayerRows, 5)
    Property_Var(int, pcBLayerCols, 1)

    //PCB扫码DataMatrix识别ROI(相机图像坐标)
    Property_Var(int, pcbScanRoiX, 0)
    Property_Var(int, pcbScanRoiY, 0)
    Property_Var(int, pcbScanRoiW, 0)
    Property_Var(int, pcbScanRoiH, 0)

    //具体托盘点位坐标
    QVector<QVector4D> feedTrayPosHolder;
    QVector<QVector4D> feedTrayPosPCB;
};

//夹爪配方
class PARAMMANAGER_EXPORT RecipeGrip : public IRecipeBase
{
    Q_OBJECT
public:
    
    Property_Var(int,grabN,50)
    Property_Var(int,rotateN,50)
};

//撕膜参数配方
class PARAMMANAGER_EXPORT RecipeFilmTear : public IRecipeBase
{
    Q_OBJECT
public:
    RecipeFilmTear &operator=(const RecipeFilmTear&);

    Property_Var(int,dProbeInitHeight,0)      //探针初始高度
    Property_Var(int,dProbeSettingHeight,0)   //探针设置高度
    Property_Var(int,dPressMin,0)
    Property_Var(int,dPressMax,0)
    Property_Var(int,dHeightMin,0)
    Property_Var(int,dHeightMax,0)
};

//脏污检测配方
class PARAMMANAGER_EXPORT RecipeDirty : public IRecipeBase
{
    Q_OBJECT
public:
    RecipeDirty& operator=(const RecipeDirty& other) {
        if (this == &other) return *this;
        modelPath = other.modelPath;
        threshold = other.threshold;
        maskThreshold = other.maskThreshold;
        enableOpenvino = other.enableOpenvino;
        intraThreads = other.intraThreads;
        filepath = other.filepath;
        listRecipe = other.listRecipe;
        curRecipe = other.curRecipe;
        return *this;
    }
    Property_Var(QString,modelPath,"")         //ONNX模型文件夹路径
    Property_Var(QString,modelPathMtr,"")       //MTR计量模型文件夹路径
    Property_Var(double,threshold,0.5)         //图像级OK/NG阈值(score>=threshold→NG)
    Property_Var(double,maskThreshold,0.3)     //像素级mask阈值
    Property_Var(int,enableOpenvino,0)         //是否启用OpenVINO加速
    Property_Var(int,intraThreads,0)           //ONNX推理线程数(0=自动)
};


class PARAMMANAGER_EXPORT LightParam : public QObject
{
    Q_OBJECT
public:
    Property_Var(QString, StrlightPort, "COM2")
    Property_Var(QString, StrlightBaud, "115200")
    Property_Var(int, channel, 0)
    Property_Var(int, lightValue, 0)
};

class PARAMMANAGER_EXPORT ScanCodeParam : public QObject
{
    Q_OBJECT
public:
    Property_Var(QString, sIP, "127.0.0.1")
};

class PARAMMANAGER_EXPORT TrayRfidDebugParam : public QObject
{
    Q_OBJECT
public:
    Property_Var(QString, comPort, "COM5")
    Property_Var(QString, StrbaudRate, "115200")
    Property_Var(int, codeLength, 100)
    Property_Var(QString, Strstation, u8"空盘缓存")
};

class PARAMMANAGER_EXPORT PressDispDebugParam : public QObject
{
    Q_OBJECT
public:
    // ========== 压力传感器参数 ==========
    Property_Var(QString, StrpressPort, "COM7")
    Property_Var(QString, StrpressBaud, "115200")
    Property_Var(int, pressDataBits, 8)
    Property_Var(int, pressParity, 0)
    Property_Var(int, pressStopBits, 1)
    Property_Var(int, pressSlaveID, 1)
    Property_Var(int, pressRespTime, 1000)
    Property_Var(int, pressRetry, 1)

    // ========== 位移传感器参数 ==========
    Property_Var(QString, StrdispPort, "COM11")
    Property_Var(QString, StrdispBaud, "9600")
    Property_Var(int, dispDataBits, 8)
    Property_Var(int, dispParity, 0)
    Property_Var(int, dispStopBits, 1)
};

// 硬件设置
class HardwareParam : public QObject
{
    Q_OBJECT
public:
    PropertyVar(QString,fileName)
    Property_Var(double,gripToCamX,0)   //上料左夹爪到视觉相机X方向间距
    Property_Var(double,gripToCamY,0)   //上料左夹爪到视觉相机Y方向间距
    Property_Var(QString,holderBoxPitch,"-46.244")   //壳体料箱层间Z间距(负值向下)
    Property_Var(QString,pcbBoxPitch,"0")   //PCB料箱层间Z间距(负值向下)
    ScanCodeParam     trayCodeParam;      //壳体扫码枪IP
    ScanCodeParam     pcbCodeParam;       //PCB扫码枪IP
    TrayRfidDebugParam trayRfidDebugParam;  //RFID调试参数
    LightParam        lightParam;         //光源参数
    PressDispDebugParam pressDispDebugParam;

    //测试盒相关
    Property_Var(int,testBoxType,0)      //测试盒类型
    PropertyVar(QStringList,testBoxSn)   //测试盒序列号
};


struct ShieldParam
{   //true代表屏蔽
    bool  safeDoor=false;  //安全门
    bool  airWaring = false;  //气压报警
    bool  vision=true;     //视觉
    bool  turntable[4]={false,false,false,false};
    bool  mes = false;  //MES
};

// 系统设置
class SystemParam : public QObject
{
    Q_OBJECT
    /*常规设置*/
public:
    SystemParam& operator=(const SystemParam&);

public:
    PropertyVar(QString,filepath)
    PropertyVar(QString,fileName)
    /*常规设置*/
    int  acupointCount=1;

   // PropertyVar(QString,serverIp)
    Property_Var(QString,preDeviceIp,"10.182.5.12")
    Property_Var(QString,nextDeviceIp,"10.255.48.21")
    ShieldParam shieldParam;
};

//模组测试信息
struct ModuleInfo
{
    bool result = true;                    //测试结果
    int  station = 0;                      //穴位
    QString pcbCode;                       //扫码结果
    QString sTime;                         //测试开始时间
    QString eTime;                         //测试结束时间

    double pressure = 0.0;                 //压力值
    double displacement = 0.0;             //位移值

    QString errorMsg;                      //失败描述
    QString ngReason;                      //NG盘显示原因
    QTime calibTime;                       //耗时统计
};

Q_DECLARE_METATYPE(ModuleInfo)
Q_DECLARE_METATYPE(std::vector<ModuleInfo>)

#endif // PARAMDEF_H
