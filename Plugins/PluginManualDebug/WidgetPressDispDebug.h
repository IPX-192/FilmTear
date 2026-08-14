#ifndef WIDGETPRESSDISPDEBUG_H
#define WIDGETPRESSDISPDEBUG_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QGroupBox>
#include <QComboBox>
#include <QSpinBox>
#include <QTextEdit>


class WidgetPressDispDebug : public QWidget
{
    Q_OBJECT
public:
    explicit WidgetPressDispDebug(QWidget *parent = nullptr);
    ~WidgetPressDispDebug() override;

public slots:
    void LoadUIParam();
    void UpdateParamToUI();
    void SaveUIParam();

private:
    QGroupBox* CreatePressSensorPanel();
    QGroupBox* CreateDispSensorPanel();
    void AppendLog(const QString& text);

public slots:
    int event_PressDispSensorReady();

private:
    QTextEdit* m_textLog;
};

#endif // WIDGETPRESSDISPDEBUG_H
