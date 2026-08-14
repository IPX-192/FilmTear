#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include "FilmTearNode.h"
#include "VisAppBus.h"
#include "VisMotorManager.h"
#include "VisMotorToolData.h"
#include "VisAppThreadPool.h"
#include "ParamManager.h"

using namespace  VisMotorToolSpace;

FilmTearNode::FilmTearNode(ModuleInfo* item,int station)
{
	m_item = item;
	m_station = station;
}

int FilmTearNode::Process()
{
	ShowLog(Log_Fixture, m_station, Log_Info, QString(u8"开始撕膜流程%1").arg(m_station + 1));
	int nRes = GrabPCB();
	if (nRes != 0)return nRes;
	nRes = FilmTear();
	if (nRes != 0)return nRes;
	nRes = PlacePCB();
	if (nRes != 0)return nRes;
	ShowLog(Log_Fixture, m_station, Log_Info, QString(u8"撕膜结束%1").arg(m_station + 1));
	return 0;
}

int FilmTearNode::GrabPCB()
{
	//移动到安全位
	int nRes = VisMotorInstance->MovePositionAbs(FilmTearGripSafe);
	if (nRes != 0)return nRes;
	//张开夹爪
	nRes = SetGripClose(false);
	if (nRes != 0)return nRes;
	//到抓取位
	nRes = VisMotorInstance->MovePositionAbs(FileTearFeed);
	if (nRes != 0)return nRes;
	//闭合夹爪
	nRes = SetGripClose(true);
	if (nRes != 0)return nRes;
	//移动到安全位
	nRes = VisMotorInstance->MovePositionAbs(FilmTearGripSafe);
	if (nRes != 0)return nRes;
	bool exist = false;
	nRes = CheckGripModuleExist(exist);
	if (nRes != 0)return nRes;
	//如果没抓到
	if (!exist) {
		ShowLog(Log_Fixture, m_station, Log_Error, QString(u8"未抓取到转盘PCB"));
		QStringList listBtn;
		listBtn << u8"重新夹取" << u8"停止生产";
		nRes = VisAppBus::sendEvent("PopupUserMsgBox", listBtn, QString(u8"未抓取到转盘PCB"));
		if (nRes == 0) {
			nRes = GrabPCB();
			if (nRes != 0)return nRes;
			return 0;
		}
		else {
			VisAppBus::sendEvent("AutoEmg");
			return HardWareErr;
		}
		return 0;
	}
	return 0;
}

int FilmTearNode::FilmTear()
{
    //到撕膜位
    int nRes = VisMotorInstance->MovePositionAbs(FileTearWork);
    if (nRes != 0)return nRes;
    //撕膜动作:旋转R1,IN49(IN_FilmTearDetect)绿灯(ON)时顺时针旋转,变红灯(OFF)表示膜已撕掉,停止旋转
    if (VisMotorInstance->GetIoInput(IN_FilmTearDetect) == IO_ON) {
        VisMotorInstance->InitUserParamSpeed(MotorFilmtearR1, 0.1);
        VisMotorInstance->MotorMoveAbs(MotorFilmtearR1, 1, false, true);
        //等待IN49变红灯(IO_OFF)后停止旋转
        VisMotorInstance->SelectIoInput(IN_FilmTearDetect, IO_OFF, 30000);
        VisMotorInstance->StopContinuous(MotorFilmtearR1);
    }
    else {
        VisMotorInstance->StopContinuous(MotorFilmtearR1);
    }
    // 获取撕膜峰值压力
    float pressure = 0.0f;
    VisAppBus::sendEvent("PressureSensorGetPeakPressure", "TearStation", pressure);
    m_item->pressure = static_cast<double>(pressure);

    // 获取撕膜位移高度
    QString sensorId;
    double height = 0.0;
    VisAppBus::sendEvent("DisplacementSensorReadHeight", "TearStation", sensorId, height);
    m_item->displacement = height;

    CheckFilmTearResult();
    SaveFilmDataToCsv();
    //到撕膜检测位
    nRes = VisMotorInstance->MovePositionAbs(FileTearDetect);
    if (nRes != 0)return nRes;
    //IN49已用于撕膜旋转判定,此处检测位判断屏蔽
    /*if (VisMotorInstance->GetIoInput(IN_FilmTearDetect) == IO_ON){
        ShowLog(Log_Fixture, m_station, Log_Error, QString(u8"撕膜失败"));
        m_item->errorMsg = u8"撕膜失败";
        m_item->ngReason = u8"撕膜失败";
    }*/

    //撕膜旋转IO 49的

    //移动到安全位
    nRes = VisMotorInstance->MovePositionAbs(FilmTearGripSafe);
    if (nRes != 0)return nRes;
    return 0;
}

int FilmTearNode::PlacePCB()
{
	//到放置位
	int nRes = VisMotorInstance->MovePositionAbs(FileTearBlank);
	if (nRes != 0)return nRes;
	//张开夹爪
	nRes = SetGripClose(false);
	if (nRes != 0)return nRes;
	//移动到安全位
	nRes = VisMotorInstance->MovePositionAbs(FilmTearGripSafe);
	if (nRes != 0)return nRes;
	bool exist = false;
	//检查模组
	nRes = CheckGripModuleExist(exist);
	if (nRes != 0)return nRes;
	if (GlobalParam->flagOffline || GlobalParam->emptyRun) {
		exist = false;
	}
	if (exist) {
		QString errInfo = QString(u8"撕膜夹爪松开后仍检测到模组");
		ShowLog(Log_Fixture, m_station, Log_Error, errInfo);
		VisAppBus::sendEvent("PopupErrInfo", errInfo);
		return HardWareErr;
	}
	return 0;
}

int FilmTearNode::SetGripClose(bool close)
{
	int nRes = 0;
	if (close) {
		//闭合夹爪:固定到0(张开按伺服配方)
		nRes = VisMotorInstance->MotorMoveAbs(MotorFilmtearGripX, 0);
		if (nRes != 0) return nRes;
		std::this_thread::sleep_for(std::chrono::milliseconds(200));
	}
	else {
		nRes = VisMotorInstance->MovePositionAbs(FilmtearGripOpen);
		if (nRes != 0) return nRes;
	}
	return 0;
}

int FilmTearNode::CheckGripModuleExist(bool& exist)
{
	if (GlobalParam->flagOffline || GlobalParam->emptyRun) {
		exist = true;
		return 0;
	}

	curSportState state;
	if (!VisMotorInstance->ReadCurSportState(MotorFilmtearGripX, state)) {
		QString errInfo = QString(u8"获取撕膜夹爪夹取状态失败");
		ShowSystemLog(Log_Error, errInfo);
		VisAppBus::sendEvent("PopupErrInfo", errInfo);
		return HardWareErr;
	}
	exist = (state == SPAORTSTOPANDNOCATCH);

	return 0;
}

int FilmTearNode::CheckFilmTearResult()
{
    const RecipeFilmTear& recipe = GlobalParam->recipeFilmTear;
    bool pressureOk = (m_item->pressure >= recipe.dPressMin && m_item->pressure <= recipe.dPressMax);
    bool heightOk   = (m_item->displacement >= recipe.dHeightMin && m_item->displacement <= recipe.dHeightMax);

    if (!pressureOk || !heightOk)
    {
        m_item->result = false;
        QStringList ngList;
        if (!pressureOk) ngList << QString(u8"压力越限:%1(%2~%3)")
                                   .arg(m_item->pressure).arg(recipe.dPressMin).arg(recipe.dPressMax);
        if (!heightOk)   ngList << QString(u8"位移越限:%1(%2~%3)")
                                   .arg(m_item->displacement).arg(recipe.dHeightMin).arg(recipe.dHeightMax);
        m_item->ngReason = QString(u8"撕膜NG:%1").arg(ngList.join(";"));
        ShowLog(Log_Fixture, m_station, Log_Error, QString(u8"撕膜检测NG:") + m_item->ngReason);
    }
    return 0;
}

int FilmTearNode::SaveFilmDataToCsv()
{
    QString productFolder = "DefaultProduct";
    if (GlobalParam->recipeProduct.curMatrix != nullptr)
    {
        productFolder = GlobalParam->recipeProduct.curMatrix->productName;
    }
    QString exePath = QCoreApplication::applicationDirPath();
    QString dataPath = exePath + "/Data/" + productFolder;

    QDir dir;
    if (!dir.exists(dataPath))
        dir.mkpath(dataPath);

    QString dateStr = QDate::currentDate().toString("yyyy-MM-dd");
    QString filePath = dataPath + "/" + dateStr + "_film.csv";

    bool fileExist = QFile::exists(filePath);
    QFile file(filePath);
    if (!file.open(QIODevice::Append))
    {
        ShowLog(Log_Fixture, m_station, Log_Error,
                QStringLiteral("CSV open failed: %1").arg(filePath));
        return -1;
    }

    const bool isNewFile = (file.size() == 0);

    if (isNewFile)
    {
        // UTF-8 BOM
        file.write("\xEF\xBB\xBF", 3);

        // 使用 Unicode 转义，避免源码按 GBK/ANSI 编译。
        const QString header = QStringLiteral(
                    "\u65F6\u95F4,"             // 时间
                    "\u6761\u7801,"             // 条码
                    "\u538B\u529B(N),"          // 压力(N)
                    "\u4F4D\u79FB(mm)\r\n");    // 位移(mm)

        file.write(header.toUtf8());
    }

    QTextStream out(&file);
    out.setCodec("UTF-8");

    const QString timeStr =
            QDateTime::currentDateTime().toString(
                QStringLiteral("yyyy-MM-dd hh:mm:ss"));

    out << timeStr << ','
        << m_item->pcbCode << ','
        << QString::number(m_item->pressure, 'f', 3) << ','
        << QString::number(m_item->displacement, 'f', 3) << "\r\n";

    out.flush();

    if (out.status() != QTextStream::Ok)
    {
        file.close();
        return -1;
    }

    file.close();
    return 0;
}
