#ifndef WIDGETLIGHTCTL_H
#define WIDGETLIGHTCTL_H

#include <QWidget>
#include <QSerialPort>
#include <QSerialPortInfo>

class WidgetLightCtl : public QWidget
{
    Q_OBJECT
public:
    explicit WidgetLightCtl(QWidget *parent = nullptr);

public slots:
    void LoadUIParam();
    void UpdateParamToUI();
    void SaveUIParam();

private:
    bool m_serialOpened = false;    // UI 构建辅助函数
    QWidget* CreateSerialConfigGroup(QString portObject, QString serBaudRateObject);
    QWidget* CreateBasicSwitch(QString title, bool isOpened);
    QWidget* CreateLightSource(QString title,QString channelObject,QString lightValueObject);

private :
	bool OpenLightSerial(const QString& portName, int baudRate);
    void CloseLightSerial();
    void SendLightCommand(int lightId, int brightness);

signals:

};

#endif // WIDGETLIGHTCTL_H
