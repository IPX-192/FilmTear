#include <QSettings>
#include <QDateTime>
#include "WearingParts.h"
#include "ui_WearingParts.h"
#include <QDebug>
#include <QMessageBox>

WearingParts::WearingParts(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WearingParts)
{
    ui->setupUi(this);
    Init();
}

WearingParts::~WearingParts()
{
    delete ui;
}

void WearingParts::SetWearingPartName(QString &name, WearingPartType type)
{
    if (LIMITFREQUENCY == type)
    {
        ui->labelFreqName->setText(name);
        ui->stackedWidget->setCurrentWidget(ui->pageFrequency);
        ui->editSetFreq->setToolTip(QString::fromLocal8Bit("%1限定次数").arg(name));
        ui->editUseFreq->setToolTip(QString::fromLocal8Bit("%1已使用次数").arg(name));
    }
    else
    {
        ui->labelTimeName->setText(name);
        ui->stackedWidget->setCurrentWidget(ui->pageTime);
        ui->editSetFreq->setToolTip(QString::fromLocal8Bit("%1限定时间").arg(name));
        ui->editUseFreq->setToolTip(QString::fromLocal8Bit("%1剩余时间").arg(name));
    }
}

void WearingParts::SetValue(int limitValue, int usedValue, WearingPartType type)
{
    if (LIMITFREQUENCY == type)
    {
        ui->editSetFreq->setText(QString::number(limitValue));
        ui->editUseFreq->setText(QString::number(usedValue));
    }
    else
    {
        ui->editSetLimitDate->setText(QString::number(limitValue));
        ui->editRemainingDate->setText(QString::number(usedValue));
    }
}

void WearingParts::SetValue(int usedValue, WearingPartType type)
{
    if (LIMITFREQUENCY == type)
    {
        ui->editUseFreq->setText(QString::number(usedValue));
    }
    else
    {
        ui->editRemainingDate->setText(QString::number(usedValue));
    }
}

void WearingParts::Init()
{
    setAttribute(Qt::WA_QuitOnClose, false);
    setWindowFlag(Qt::FramelessWindowHint);
    connect(ui->btnSetFreq, SIGNAL(clicked()), this, SLOT(SetFreqLimitValue()));
    connect(ui->btnResetFreq, SIGNAL(clicked()), this, SLOT(ResetFreqUsedValue()));
    connect(ui->btnSetLimitDate, SIGNAL(clicked()), this, SLOT(SetTimeLimitValue()));
    connect(ui->btnResetTime, SIGNAL(clicked()), this, SLOT(ResetTimeUsedValue()));
}

void WearingParts::SetFreqLimitValue()
{
    QSettings *settings = new QSettings(m_consumables, QSettings::NativeFormat);
    if (QMessageBox::Yes == QMessageBox::question(nullptr, QString::fromLocal8Bit("确认修改？"), QString::fromLocal8Bit("修改限制次数？")))
    {
        QString beforeModify = settings->value(ui->labelFreqName->text() + "@" +
                                           QString::number(LIMITFREQUENCY) + "/" + "limit").toString();
        settings->setValue(ui->labelFreqName->text() + "@" + QString::number(LIMITFREQUENCY) + "/" + "limit",
                           ui->editSetFreq->text());
        emit operationLog(ui->labelFreqName->text() + QString::fromLocal8Bit(",次数限制被修改, 修改后:%1, 修改前:%2,次").
                          arg(ui->editSetFreq->text()).arg(beforeModify));
        QMessageBox::information(nullptr, QString::fromLocal8Bit("提示"), QString::fromLocal8Bit("修改成功"));
    }
    else
    {
        ui->editSetFreq->setText(settings->value(ui->labelFreqName->text() + "@" +
                                                 QString::number(LIMITFREQUENCY) + "/" + "limit").toString());
    }
    delete settings;
    settings = nullptr;
}

void WearingParts::ResetFreqUsedValue()
{
    if (QMessageBox::Yes == QMessageBox::question(nullptr, QString::fromLocal8Bit("确认重置？"), QString::fromLocal8Bit("重置使用次数？")))
    {
        QSettings *settings = new QSettings(m_consumables, QSettings::NativeFormat);
        QString text = ui->labelFreqName->text() + QString::fromLocal8Bit(", 使用次数被重置,重置后次数:%1, 重置前次数:%2,次").arg(0).
                arg(settings->value(ui->labelFreqName->text() + "@" + QString::number(LIMITFREQUENCY) + "/" + "used").toString());
        ui->editUseFreq->setText("0");
        settings->setValue(ui->labelFreqName->text() + "@" + QString::number(LIMITFREQUENCY) + "/" + "used", 0);
        delete settings;
        settings = nullptr;
        emit operationLog(text);
        QMessageBox::information(nullptr, QString::fromLocal8Bit("提示"), QString::fromLocal8Bit("重置成功"));
    }
}

void WearingParts::SetTimeLimitValue()
{
    QSettings *settings = new QSettings(m_consumables, QSettings::NativeFormat);
    if (QMessageBox::Yes == QMessageBox::question(nullptr, QString::fromLocal8Bit("确认修改？"), QString::fromLocal8Bit("修改检修时间？")))
    {
        QString beforeModify = settings->value(ui->labelTimeName->text() + "@" +
                                               QString::number(LIMITTIME) + "/" + "limit").toString().split("@").last();
        QString timeStringFile =
                settings->value(ui->labelTimeName->text() + "@" + QString::number(LIMITTIME) + "/" + "limit").
                toString().split("@").first();
        settings->setValue(ui->labelTimeName->text() + "@" + QString::number(LIMITTIME) + "/" + "limit",
                           timeStringFile + "@" + ui->editSetLimitDate->text());
        emit operationLog(ui->labelTimeName->text() + QString::fromLocal8Bit(",检修时间被修改, 修改后:%1, 修改前:%2,天").
                          arg(ui->editSetLimitDate->text()).arg(beforeModify));
        QMessageBox::information(nullptr, QString::fromLocal8Bit("提示"), QString::fromLocal8Bit("修改成功"));
    }
    else
    {
        ui->editSetLimitDate->setText(settings->value(ui->labelTimeName->text() + "@" +
                                                 QString::number(LIMITTIME) + "/" + "limit").toString().
                                 split("@").last());
    }
    delete settings;
    settings = nullptr;
}

void WearingParts::ResetTimeUsedValue()
{
    if (QMessageBox::Yes == QMessageBox::question(nullptr, QString::fromLocal8Bit("确认重置？"), QString::fromLocal8Bit("重置检修时间？")))
    {
        QDateTime currentTimeFile = QDateTime::currentDateTime();
        QString timeStringFile = currentTimeFile.toString("yyyy-MM-dd");
        QSettings *settings = new QSettings(m_consumables, QSettings::NativeFormat);
        QString text = ui->labelTimeName->text() + QString::fromLocal8Bit(", 待检修时间被重置, 重置后时间:%1, 重置前时间:%2").
                arg( settings->value(ui->labelTimeName->text() + "@" + QString::number(LIMITTIME) + "/" + "used").toString()).
                arg( settings->value(ui->labelTimeName->text() + "@" + QString::number(LIMITTIME) + "/" + "limit").toString());
        ui->editRemainingDate->setText(ui->editSetLimitDate->text());
        settings->setValue(ui->labelTimeName->text() + "@" + QString::number(LIMITTIME) + "/" + "limit",
                           timeStringFile + "@" + ui->editSetLimitDate->text());
        settings->setValue(ui->labelTimeName->text() + "@" + QString::number(LIMITTIME) + "/" + "used",
                           ui->editSetLimitDate->text());
        delete settings;
        settings = nullptr;
        emit operationLog(text);
        QMessageBox::information(nullptr, QString::fromLocal8Bit("提示"), QString::fromLocal8Bit("重置成功"));
    }
}
