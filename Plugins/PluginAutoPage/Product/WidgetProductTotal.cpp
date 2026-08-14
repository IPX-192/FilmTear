#include "WidgetProductTotal.h"
#include "ui_WidgetProductTotal.h"
#include <QTabWidget>
#include <QGridLayout>
#include <QDateTime>
#include <QSettings>
#include <QTextCodec>
#include "VisAppBus.h"
#include "WidgetStationProduct.h"

QMap<int, int> operator+(const QMap<int, int>& lhs, const QMap<int, int>& rhs)
{
	QMap<int, int> result = lhs;
	for (auto it = rhs.constBegin(); it != rhs.constEnd(); ++it) {
		result[it.key()] += it.value();
	}
	return result;
}

QMap<QString, int> operator+(const QMap<QString, int>& lhs, const QMap<QString, int>& rhs)
{
	QMap<QString, int> result = lhs;
	for (auto it = rhs.constBegin(); it != rhs.constEnd(); ++it) {
		result[it.key()] += it.value();
	}
	return result;
}

WidgetProductTotal::WidgetProductTotal(int stationNum, QString filepath,QWidget *parent) :
    m_stationNum(stationNum),
	m_filePath(filepath),
    QWidget(parent),
    ui(new Ui::WidgetProductTotal)
{
    ui->setupUi(this);
    InitWidget();
	LoadUIParam();
	CalcuTotal();
	for (int i = 0; i < ui->tabWidget->count(); i++) {
		WidgetStationProduct* widget = (WidgetStationProduct*)ui->tabWidget->widget(i);
		widget->UpdateWidget();
	}
	
    VisAppBus::subscibeEvent(this,"UpdateProduct");
}

WidgetProductTotal::~WidgetProductTotal()
{
    delete ui;
}

void WidgetProductTotal::LoadUIParam()
{
	QSettings setting(m_filePath + "/product.ini", QSettings::IniFormat);
	setting.setIniCodec(QTextCodec::codecForName("UTF-8")); //重点
	for (int i = 1; i <= m_stationNum; ++i) {
		//良率信息
		setting.beginGroup(QString("Station%1").arg(i));
		m_productInfo[i].numTotal = setting.value("numTotal").toInt();
		m_productInfo[i].numOK = setting.value("numOK").toInt();
		m_productInfo[i].numNG = setting.value("numNG").toInt();
		m_productInfo[i].timeCT = setting.value("timeCT").toDouble();
		setting.endGroup();
		//NG信息
		setting.beginGroup(QString("NGSort%1").arg(i));
		QStringList listKey = setting.allKeys();
		for (int j = 0; j < listKey.size(); j++) {
			QString value = setting.value(listKey.at(j)).toString();
			QStringList listValue = value.split(",");
			QString key = listValue.at(0);
			m_productInfo[i].ngInfo[key] = listValue.at(1).toInt();
		}
		setting.endGroup();
		//小时产量
		setting.beginGroup(QString("HourNum%1").arg(i));
		listKey = setting.allKeys();
		for (int j = 0; j < listKey.size(); j++) {
			QString key = listKey.at(j);
			m_productInfo[i].numHour[key.toInt()] = setting.value(key).toInt();
		}
		setting.endGroup();
	};
}

void WidgetProductTotal::SaveUIParam()
{
	QSettings setting(m_filePath + "/product.ini", QSettings::IniFormat);
	setting.setIniCodec("UTF-8");
    setting.clear();
	for (int i = 1; i <= m_stationNum; ++i) {
		//良率信息
		setting.beginGroup(QString("Station%1").arg(i));
		setting.setValue("numTotal", m_productInfo[i].numTotal);
		setting.setValue("numOK", m_productInfo[i].numOK);
		setting.setValue("numNG", m_productInfo[i].numNG);
		setting.setValue("timeCT", m_productInfo[i].timeCT);
		setting.endGroup();
		//NG信息
		setting.beginGroup(QString("NGSort%1").arg(i));
		int ngNum = 1;
		for (auto it = m_productInfo[i].ngInfo.begin(); it != m_productInfo[i].ngInfo.end(); ++it) {
			QString value = it.key() + QString(",%1").arg(it.value());
			setting.setValue(QString("NG%1").arg(ngNum++), value);
		}
		setting.endGroup();
		//小时产量
		setting.beginGroup(QString("HourNum%1").arg(i));
		for (auto it = m_productInfo[i].numHour.begin(); it != m_productInfo[i].numHour.end(); ++it) {
			setting.setValue(QString::number(it.key()), it.value());
		}
		setting.endGroup();
	};
	setting.sync();
}

void WidgetProductTotal::InitWidget()
{
    for (int i = 0; i <= m_stationNum; i++) {
        m_productInfo[i] = StationInfo();
        m_productInfo[i].station = i;
        m_productInfo[i].ptrTotal = &m_productInfo[0];
        for (int j = 0; j < 24; j++) {
			m_productInfo[i].numHour[j] = 0;
        }
        QString tabName = (i == 0) ? u8"总工位" : (QString(u8"工位%1").arg(i));
		WidgetStationProduct* widget = new WidgetStationProduct(&m_productInfo[i], ui->tabWidget);
        ui->tabWidget->addTab(widget, tabName);
		connect(widget, &WidgetStationProduct::sigClearData, this, &WidgetProductTotal::slotClearData);
    }
}

void WidgetProductTotal::CalcuTotal()
{
	StationInfo totalStation;
    totalStation.station = 0;
    totalStation.ptrTotal = &m_productInfo[0];
    for (int i = 1; i <= m_stationNum; i++) {
        totalStation.numTotal += m_productInfo[i].numTotal;
        totalStation.numOK += m_productInfo[i].numOK;
        totalStation.numNG += m_productInfo[i].numNG;
        if (m_productInfo[i].numTotal != 0 ) {
			if (totalStation.timeCT == 0)
				totalStation.timeCT = m_productInfo[i].timeCT;
			else
				totalStation.timeCT = (totalStation.timeCT + m_productInfo[i].timeCT) / 2;
        }
		totalStation.numHour = totalStation.numHour + m_productInfo[i].numHour;
        totalStation.ngInfo = totalStation.ngInfo + m_productInfo[i].ngInfo;
    }
    m_productInfo[0] = totalStation;
}

void WidgetProductTotal::ResetHourData()
{
	//计算前12小时key
	int hour = QTime::currentTime().hour();
	QVector<int> targetKey;
	int hoursShow = 12;
	for (int i = hoursShow - 1; i >= 0; --i) {
		int targetHour = (hour - i + 24) % 24;
		targetKey << targetHour;
	}
	//其他时间段置0
	for (int i = 1; i <= m_stationNum; ++i) {
		for (auto it = m_productInfo[i].numHour.begin(); it != m_productInfo[i].numHour.end(); ++it) {
			if (!targetKey.contains(it.key())) {
				it.value() = 0;
			}
		}
	}
}

int WidgetProductTotal::event_UpdateProduct(ProductTask taskItem)
{
    int station = taskItem.station + 1;
    int hour= QTime::currentTime().hour();
	ResetHourData();
    m_productInfo[station].numTotal += 1;
    m_productInfo[station].numHour[hour] += 1;
	if (taskItem.result) {
		m_productInfo[station].timeCT = taskItem.ctTime;
		m_productInfo[station].numOK += 1;
	}
    else {
        m_productInfo[station].numNG += 1;
        m_productInfo[station].ngInfo[taskItem.errItem] += 1;
    }
    CalcuTotal();
	//更新界面
    WidgetStationProduct* widget = (WidgetStationProduct*)ui->tabWidget->widget(station);
    widget->UpdateWidget();
	widget = (WidgetStationProduct*)ui->tabWidget->widget(0);
	widget->UpdateWidget();
	SaveUIParam();

    return 0;
}

void WidgetProductTotal::slotClearData(int station, bool all)
{
	if (station != 0 && !all) {
		m_productInfo[station] = StationInfo();
		m_productInfo[station].station = station;
		m_productInfo[station].ptrTotal = &m_productInfo[0];
	}
	else {
		StationInfo* ptrTotal = &m_productInfo[0];
		for (int i = 0; i <= m_stationNum; i++) {
			m_productInfo[i] = StationInfo();
			m_productInfo[i].station = i;
			m_productInfo[i].ptrTotal = ptrTotal;
		}
	}
	CalcuTotal();
	for (int i = 0; i < ui->tabWidget->count(); i++) {
		WidgetStationProduct* widget = (WidgetStationProduct*)ui->tabWidget->widget(i);
		widget->UpdateWidget();
	}
	SaveUIParam();
}


