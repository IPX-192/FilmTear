#include "ScanCodeClient.h"
#include <QTime>
#include <QApplication>
#include "VisAppBus.h"
#include "ParamManager.h"

#define SendOnCmd "LON\r\n"
#define SendOffCmd "LOFF\r\n"

ScanCodeClient::ScanCodeClient(QObject *parent) : QObject(parent)
{
    m_initThreadID=std::this_thread::get_id();
	VisAppBus::subscibeEvent(this, "Connect");
	VisAppBus::subscibeEvent(this, "Disconnected");
    VisAppBus::subscibeEvent(this,"GetBarCode");
	Init();
}

void ScanCodeClient::SetName(QString name)
{
    m_name=name;
}

int ScanCodeClient::Connect(QString ipAddress)
{
	if (m_connectFlag) {
		m_tcpClient->disconnectFromHost();
	}
	QString portValue = "9004";
	m_tcpClient->connectToHost(ipAddress, portValue.toUShort());
	m_connectFlag = m_tcpClient->waitForConnected(5000);
    ShowSystemLog(m_connectFlag ? Log_Info : Log_Error, QString(u8"%1扫码枪连接%2").arg(m_name).arg(m_connectFlag ? u8"成功" : u8"失败"));
	return m_connectFlag ? 0 : -1;
}

int ScanCodeClient::Disconnected()
{
	m_tcpClient->disconnectFromHost();
	m_connectFlag = false;
    ShowSystemLog(Log_Info, QString(u8"%1扫码枪断开连接").arg(m_name));
	return 0;
}

void ScanCodeClient::Init()
{
    m_tcpClient = new QTcpSocket(this);             //实例化tcpClient
    m_tcpClient->abort();                           //取消原有连接
    connect(m_tcpClient, SIGNAL(readyRead()), this, SLOT(ReadData()));
}

int ScanCodeClient::event_Connect(QString name, QString ipAddress)
{
	if (m_name != name)return 0;
	return Connect(ipAddress);
}

int ScanCodeClient::event_Disconnected(QString name)
{
	if (m_name != name)return 0;
	return Disconnected();
}

void ScanCodeClient::ReadData()
{
    QByteArray buffer = m_tcpClient->readAll();
    m_barCode += QString(buffer);
    m_replyFlag = true;        //条码获取完成
}

int ScanCodeClient::event_GetBarCode(QString name,QString& barCode)
{
    if(m_name!=name)return 0;
    barCode="";
    if (!m_connectFlag)
        return -1;
    m_barCode="";
    int scanCodeMax=m_scanCodeMax;
    while(scanCodeMax){
        bool bRet=false;
        //开扫码枪
        m_replyFlag=false;
        if (std::this_thread::get_id() == m_initThreadID){
            bRet = slotSendData(SendOnCmd);
            if(!bRet)return -1;
        }
        else {
            QMetaObject::invokeMethod(this, "slotSendData", Qt::BlockingQueuedConnection
                , Q_RETURN_ARG(bool, bRet)
                ,Q_ARG(QString ,SendOnCmd));
            if(!bRet)return -1;
        }
        bRet=WaitTime();
        //关扫码枪
        if (std::this_thread::get_id() == m_initThreadID){
            slotSendData(SendOffCmd);
        }
        else {
            QMetaObject::invokeMethod(this, "slotSendData", Qt::BlockingQueuedConnection
                , Q_RETURN_ARG(bool, bRet)
                ,Q_ARG(QString ,SendOffCmd));
        }
        if(bRet){
            barCode=m_barCode.trimmed();
            return 0;
        }
        scanCodeMax--;
    }
    return 0;
}

bool ScanCodeClient::slotSendData(QString str)
{
    QByteArray sendText = str.toLocal8Bit();
    m_tcpClient->write(sendText);
    return m_tcpClient->flush();
}

bool ScanCodeClient::WaitTime()
{
    QTime timewait;
    timewait.start();
    while(timewait.elapsed()<3000){
        if(m_replyFlag){
            //固定等待500ms获取条码，防止数据沾包，条码不全
            QTime time;
            time.start();
            while(time.elapsed()<500){
               QApplication::processEvents();
            }
            QString barCode=m_barCode;
            if (!barCode.isEmpty() && !barCode.contains("ER")) {
                barCode = barCode.trimmed();
                return true;
            }
        }
        QApplication::processEvents();
    }
   // m_errInfo=QStringLiteral("等待接收数据超时");
    return false;
}

