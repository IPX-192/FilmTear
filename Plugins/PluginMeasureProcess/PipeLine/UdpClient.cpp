#include "UdpClient.h"
#include <QApplication>
#include <QtCore/QMetaType>
#include <QDebug>
#include "VisAppBus.h"
#include "VisAppThreadPool.h"
#include "VisMotorManager.h"
#include "ParamManager.h"
#include "VisMotorToolData.h"
#include "CylinderCtrl.h"
#include "../PluginAutoPage/WidgetFlowState.h"

using namespace  VisMotorToolSpace;
Q_DECLARE_METATYPE(UpStreamClientStatus)

UdpClient::UdpClient(QObject *parent) : QObject(parent)
{
    m_initThreadID = std::this_thread::get_id();
    m_uSocket = new QUdpSocket(this);
    connect(m_uSocket, &QUdpSocket::readyRead, this, &UdpClient::slotUdpReceive);
    m_uSocket->bind(UdpClientPort, QUdpSocket::ShareAddress);

    m_timer = new QTimer(this);
    m_timer->setInterval(200);
	connect(m_timer, &QTimer::timeout, this, &UdpClient::slotTimeOut);
    m_statusNotifyTimer = new QTimer(this);
    m_statusNotifyTimer->setInterval(200);
    connect(m_statusNotifyTimer, &QTimer::timeout, this, &UdpClient::slotNotifyFullStatus);
    m_statusNotifyTimer->start();
    VisAppBus::subscibeEvent(this, "SetBackFlowReady");
    VisAppBus::subscibeEvent(this, "SetPipeLineInStatus");
}

bool UdpClient::Start()
{
	//获取回流线缓存托盘是否有无
     
    m_timer->start();
    return true;
}

void UdpClient::slotTimeOut()
{
     SendState();
}

bool UdpClient::SendState()
{
    if (std::this_thread::get_id() == m_initThreadID) {
            return slotPostData();
    }
	else {
		bool bRet = false;
		QMetaObject::invokeMethod(this, "slotPostData", Qt::BlockingQueuedConnection
			, Q_RETURN_ARG(bool, bRet));
		return bRet;
	}

	return true;
}

bool UdpClient::slotPostData()
{
	int state = m_trayRequest;    //要料信号
	int stateValue = (state << 3);
	state = m_trayInCurPos;       //输送线载具到位
    stateValue += (state << 4);
	state = m_existTrayBackFlow;  //回流线有料
    stateValue += (state << 7);
	if (!GlobalParam->autoRunning)
		stateValue = 0;

	QString sendData = QString("IO=%1;").arg(stateValue);
	QString ip = GlobalParam->systemParam.preDeviceIp;
    quint16 port = UdpServerPort;
	qint64 nRes = m_uSocket->writeDatagram(sendData.toLocal8Bit(), QHostAddress(ip), port);
	m_uSocket->flush();

    return true;
}

void UdpClient::slotNotifyFullStatus()
{
    UpStreamClientStatus status;
    status.m_trayRequest = m_trayRequest;
    status.m_trayInCurPos = m_trayInCurPos;
    status.m_existTrayBackFlow = m_existTrayBackFlow;
    status.m_preExistTray = m_preExistTray;
    status.m_preRequestBackFlow = m_preRequestBackFlow;
    status.m_trayInNextPosBackFlow = m_trayInNextPosBackFlow;
    status.m_transTray = m_transTray;
    status.m_transTrayBackFlow = m_transTrayBackFlow;
    VisAppBus::postEvent("PipeLineClientStatusNotify", status);
}

void UdpClient::slotUdpReceive()
{
    QByteArray data;
    QHostAddress address;
    quint16 port;
    while (m_uSocket->hasPendingDatagrams()) {
        data.resize(m_uSocket->pendingDatagramSize());
        m_uSocket->readDatagram(data.data(), data.size(), &address, &port);
    }

    // 用 fromLatin1() 或 fromUtf8() 解码，因为发送的是 ASCII
    QString recvStr = QString::fromLatin1(data);
    QStringList infoList = recvStr.split(';');
    if (infoList.isEmpty())
        return;

    // 解析状态值
    QString stateInfo = infoList.at(0);
    int stateValue = stateInfo.right(stateInfo.length() - stateInfo.indexOf("=") - 1).toInt();
    m_preExistTray = stateValue & 1;                     // 上站有料
    m_preRequestBackFlow = (stateValue >> 1) & 1;        // 回流线要料
    m_trayInNextPosBackFlow = (stateValue >> 2) & 1;     // 回流线托盘到上站

    // 解析托盘信息（如果有）
    TrayInfo trayInfo;
    if (infoList.size() > 1) {
        QString trayBase64 = infoList.at(1);            // 取第二部分
        // 【关键改动】Base64 解码回二进制数据
        QByteArray trayData = QByteArray::fromBase64(trayBase64.toLatin1());

        QDataStream in(&trayData, QIODevice::ReadOnly);
        in.setVersion(QDataStream::Qt_5_13);
        in >> trayInfo;
    }
}

void UdpClient::TransTrayToCur(TrayInfo trayInfo)
{
	//根据机台适配动作

    m_trayRequest=false;
    m_trayInCurPos=true;
    m_transTray=false;
    sigPipeLineTrayReady(trayInfo);
}

void UdpClient::TransBackFlowTray()
{
	//根据机台适配动作

    m_existTrayBackFlow = true;
    m_transTrayBackFlow = false;
    sigSetBackFlowStatus(false);
}

void UdpClient::sigPipeLineTrayReady(TrayInfo trayInfo)
{
	VisAppBus::sendEventDirect("PipeLineTrayReady", trayInfo);
}

void UdpClient::sigSetBackFlowStatus(bool exist)
{
    VisAppBus::sendEventDirect("SetBackFlowStatus", exist);
}

int UdpClient::event_SetPipeLineInStatus(bool exist)
{
    m_trayRequest = !exist;
    m_trayInCurPos = exist;

    return 0;
}

int UdpClient::event_SetBackFlowReady(bool exist)
{
    m_existTrayBackFlow = exist;
    m_transTrayBackFlow = false;
    return 0;
}

