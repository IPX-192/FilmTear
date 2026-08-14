#include "UdpServer.h"
#include <QApplication>
#include <QHostInfo>
#include <QDebug>
#include <QtCore/QMetaType>
#include "VisAppBus.h"
#include "VisAppThreadPool.h"
#include "VisMotorManager.h"
#include "VisMotorToolData.h"
#include "ParamManager.h"
#include "CylinderCtrl.h"
#include "../Common/PipeLineDef.h"

using namespace  VisMotorToolSpace;
Q_DECLARE_METATYPE(DownStreamServerStatus)

UdpServer::UdpServer(QObject *parent) : QObject(parent)
{
    m_initThreadID = std::this_thread::get_id();
	m_uSocket = new QUdpSocket(this);
	connect(m_uSocket, &QUdpSocket::readyRead, this, &UdpServer::slotUdpReceive);

	m_timer = new QTimer(this);
    m_timer->setInterval(200);
	connect(m_timer, &QTimer::timeout, this, &UdpServer::slotTimeOut);
    m_statusNotifyTimer = new QTimer(this);
    m_statusNotifyTimer->setInterval(200);
    connect(m_statusNotifyTimer, &QTimer::timeout, this, &UdpServer::slotNotifyFullStatus);
    m_statusNotifyTimer->start();
	VisAppBus::subscibeEvent(this, "SetPipeLineOutStatus");
	VisAppBus::subscibeEvent(this, "SetBackFlowStatus");
    Start();
}

bool UdpServer::Start()
{
    IOLevel level = VisMotorInstance->GetIoInput(IN_PipeLineBackFlowBuf);
    m_requestTrayBackFlow = (level == IO_OFF);
    m_timer->start();
    m_uSocket->close();
    return  m_uSocket->bind(UdpServerPort, QUdpSocket::ShareAddress);
}

bool UdpServer::SendState()
{
    if (std::this_thread::get_id() == m_initThreadID) {
            return slotPostState();
        }
    else {
        bool bRet = false;
        QMetaObject::invokeMethod(this, "slotPostState", Qt::BlockingQueuedConnection
            , Q_RETURN_ARG(bool, bRet));
        return bRet;
    }

    return true;
}

bool UdpServer::slotPostState()
{
    //设置流线状态
    int statusValue = m_existTray;                      //本站有料
    int value = m_requestTrayBackFlow;                  //回流线要料
    statusValue += (value << 1);
    statusValue += (m_trayToBackFlow << 2);             //回流线到位
    //查询结束生产标志
    bool productEnd = false;
    VisAppBus::sendEvent("GetProductEnd", productEnd);
    m_trayInfo.productEnd = productEnd;
    //序列化托盘信息
    QByteArray sendTrayData;
    QDataStream out(&sendTrayData, QIODevice::WriteOnly);
    // 【关键】：设置版本号，确保发送端和接收端的解析方式一致
    out.setVersion(QDataStream::Qt_5_13);
    out << m_trayInfo;

    // 【关键改动】将二进制数据转为 Base64 字符串
    QString trayBase64 = sendTrayData.toBase64();

    // 拼装数据：IO状态 + Base64编码的托盘数据 + 结束分号
    QString sendData = QString("IO=%1;").arg(statusValue) + trayBase64 + ";";

    QString ip = GlobalParam->systemParam.nextDeviceIp;
    quint16 port = UdpClientPort;
    // 使用 toLatin1() 或 toUtf8()，因为 Base64 仅包含 ASCII 字符
    qint64 nRes = m_uSocket->writeDatagram(sendData.toLatin1(), QHostAddress(ip), port);
    m_uSocket->flush();
    return true;
}

//bool UdpServer::slotPostState()
//{
//    //设置流线状态
//    int statusValue = true;                      //本站有料
//    int value = m_requestTrayBackFlow;                  //回流线要料
//    statusValue += (value << 1);
//	statusValue += (m_trayToBackFlow << 2);             //回流线到位
//	if (!GlobalParam->autoRunning)
//        statusValue = 0;
//	//序列化托盘信息
//	QByteArray sendTrayData;
//	QDataStream out(&sendTrayData, QIODevice::WriteOnly);
//	// 【关键】：设置版本号，确保发送端和接收端的解析方式一致
//	out.setVersion(QDataStream::Qt_5_13);
//	out << m_trayInfo;

//    QString sendData = QString("IO=%1;").arg(statusValue) + sendTrayData + ";";
//    QString ip = GlobalParam->systemParam.nextDeviceIp;
//    quint16 port = UdpClientPort;
//    qint64 nRes = m_uSocket->writeDatagram(sendData.toLocal8Bit(), QHostAddress(ip), port);
//    m_uSocket->flush();

//    return true;
//}

void UdpServer::slotNotifyFullStatus()
{
    DownStreamServerStatus status;
    status.m_existTray = m_existTray;
    status.m_requestTrayBackFlow = m_requestTrayBackFlow;
    status.m_trayToBackFlow = m_trayToBackFlow;
    status.m_nextRequest = m_nextRequest;
    status.m_nextBackFlowExist = m_nextBackFlowExist;
    status.m_trayInNextPos = m_trayInNextPos;
    status.m_transTray = m_transTray;
    status.m_transTrayBackFlow = m_transTrayBackFlow;
    VisAppBus::postEvent("PipeLineServerStatusNotify", status);
}

void UdpServer::sigPipeLineOutTransEnd()
{
    VisAppBus::sendEventDirect("PipeLineOutTransEnd");
}

void UdpServer::sigSetBackFlowReady(bool exist)
{
    VisAppBus::sendEventDirect("SetBackFlowReady", exist);
}

void UdpServer::slotTimeOut()
{
    SendState();
}

void UdpServer::slotUdpReceive()
{
    QByteArray data;
    QHostAddress address;
    quint16 port;
    while (m_uSocket->hasPendingDatagrams()) {
        data.resize(m_uSocket->pendingDatagramSize());
        m_uSocket->readDatagram(data.data(), data.size(), &address, &port);
    }
    QStringList infoList = QString::fromLocal8Bit(data).split(";");
    if (!infoList.size())return;
    QString stateInfo = infoList.at(0);
    int stateValue = stateInfo.right(stateInfo.length() - stateInfo.indexOf("=") - 1).toInt();
    m_nextRequest = (stateValue >> 3) & 1;             //下游设备要料
    m_trayInNextPos = (stateValue >> 4) & 1;           //载具托盘到达下一站
    m_nextBackFlowExist = (stateValue >> 7) & 1;       //下游回流线有盘
    if (m_nextRequest && m_existTray && !m_transTray) {
        test ++;
        m_transTray = true;
        m_trayInNextPos = false;
        ShowLog(Log_PipeLineOnline, 0, Log_Info, QString(u8"托盘准备到下一站"));
        GlobalThreadPool->Commit(std::bind(&UdpServer::TransTrayToNext, this));

    }
    if (m_nextBackFlowExist && m_requestTrayBackFlow && !m_transTrayBackFlow) {
        m_transTrayBackFlow = true;
        m_trayToBackFlow = false;
        ShowLog(Log_PipeLineOnline, 0, Log_Info, QString(u8"回流托盘准备到本站"));
        GlobalThreadPool->Commit(std::bind(&UdpServer::TransTrayToCur, this));
    }

    if (infoList.size() < 2)return;
    QByteArray trayData= infoList.at(1).toLocal8Bit();
    TrayInfo  trayInfo;
    QDataStream in(&trayData, QIODevice::ReadOnly);
    in.setVersion(QDataStream::Qt_5_13);
    in >> trayInfo;
}

void UdpServer::TransTrayToNext()
{
	m_transTray = true;
	m_trayInNextPos = false;
	//下降顶升
	int nRes = CylinderCtrl::instance()->event_SetPushUp(FeedTurntable, false);
    if (nRes != 0)return;
	//下降阻挡
	nRes = CylinderCtrl::instance()->event_SetBlockUp(FeedTurntable, false);
    if (nRes != 0)return;
	//启动输送流水线
    nRes = VisAppBus::sendEventDirect("SetPipeLineMove", true);
    if (nRes != 0)return;
	
	ShowLog(Log_PipeLineOnline, 0, Log_Info, QString(u8"等待下游设备回复载具到位信号"));
	QTime time;
	time.start();
	//15秒内等待下游回复
	while (time.elapsed() < 30000) {
		if (VisMotorInstance->IsEmgStop())return;
		if (time.elapsed() > 5000 && (GlobalParam->flagOffline || GlobalParam->emptyRun)) {
			m_trayInNextPos = true;
			break;
		}
		if (m_trayInNextPos)break;
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
	if (!m_trayInNextPos) {
		ShowLog(Log_PipeLineOnline, 0, Log_Error, QString(u8"等待下游设备回复载具到位超时"));
	}
	//上升阻挡
	nRes = CylinderCtrl::instance()->event_SetBlockUp(FeedTurntable, true);
	if (nRes != 0)return;
	//停止流水线
    nRes = VisAppBus::sendEventDirect("SetPipeLineMove", false);
	if (nRes != 0) return;
    ShowLog(Log_PipeLineOnline, 0, Log_Info, QString(u8"载具到下游传输结束"));
	
	m_existTray = false;
	m_transTray = false;
	sigPipeLineOutTransEnd();
	//空跑/离线:无下游设备,模拟空载具回流到空盘缓存位,维持载具循环
	if (GlobalParam->flagOffline || GlobalParam->emptyRun) {
		sigSetBackFlowReady(true);
	}
}

void UdpServer::TransTrayToCur()
{
    //启动回流线
	int nRes = VisMotorInstance->SetIoOutput(Out_PipeLineBackFlowMotorL, IO_ON);
	if (nRes != 0)return ;
    //等待出口检测到托盘
    if (!VisMotorInstance->SelectIoInput(IN_PipeLineBackFlowBuf, IO_ON, 30000)) {
        QString errInfo = QString(u8"30秒内回流线托盘缓存没有检测到托盘信号");
        ShowLog(Log_PipeLineOnline, 0, Log_Error, errInfo);
        VisAppBus::sendEvent("PopupErrInfo", errInfo);
    }
    else {
        ShowLog(Log_PipeLineOnline, 0, Log_Info, QString(u8"回流线托盘达到本站"));
    }
    //停止流线
	nRes = VisMotorInstance->SetIoOutput(Out_PipeLineBackFlowMotorL, IO_OFF);
	if (nRes != 0)return;
    m_trayToBackFlow = true;
    m_transTrayBackFlow = false;
	
    sigSetBackFlowReady(true);
}

int UdpServer::event_SetPipeLineOutStatus(bool exist, TrayInfo trayInfo)
{
    m_trayInfo = trayInfo;
    m_existTray = exist;
	if (GlobalParam->flagOffline || GlobalParam->emptyRun) {
        test ++;
		ShowLog(Log_PipeLineOnline, 0, Log_Info, QString(u8"托盘准备到下一站"));
		GlobalThreadPool->Commit(std::bind(&UdpServer::TransTrayToNext, this));
	}

    return 0;
}

int UdpServer::event_SetBackFlowStatus(bool exist)
{
    m_requestTrayBackFlow = !exist;
    m_trayToBackFlow = exist;
    return 0;
}

