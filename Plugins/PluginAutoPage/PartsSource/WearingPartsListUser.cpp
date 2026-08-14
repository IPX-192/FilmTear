#include <QDateTime>
#include <QSettings>
#include "WearingPartsListUser.h"
#include "ui_WearingPartsListUser.h"
#include <QDebug>

WearingPartsListUser::WearingPartsListUser(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WearingPartsListUser)
{
    ui->setupUi(this);
    Init();
}

WearingPartsListUser::~WearingPartsListUser()
{
    //先删除以前的数据
    {
        for (int i = 0; i < ui->tableWidget->rowCount(); i++)
        {
            for (int j = 0; j < ui->tableWidget->columnCount(); j++)
            {
                QTableWidgetItem *item = ui->tableWidget->item(i, j);
                if (item)
                {
                    delete item;
                    item = nullptr;
                }
            }
        }
        ui->tableWidget->clear();
    }
    delete ui;
}

void WearingPartsListUser::UpdateList(QString model, bool updateUi)
{
    QSettings *settings = new QSettings(m_consumables, QSettings::NativeFormat);
    QStringList partKeys = settings->allKeys();
    //删除多余的键
    for (int i = 0; i < partKeys.size(); i++)
    {
        partKeys[i] = partKeys[i].split("/").first();
    }
    for (int i = 0; i < partKeys.count(); i++)
    {
        for (int k = i + 1; k <  partKeys.count(); k++)
        {
            if ( partKeys.at(i) ==  partKeys.at(k))
            {
                partKeys.removeAt(k);
                k--;
            }
        }
    }
    if (ui->tableWidget->rowCount() != partKeys.size() || updateUi)
    {
        delete settings;
        settings = nullptr;
        InitList(model);
    }
    else
    {
        for (int i = 0; i < partKeys.size(); i++)
        {
            if (2 == partKeys[i].split("@").size())
            {
                if (LIMITFREQUENCY == partKeys[i].split("@").last().toInt())
                {
                    m_useNumber[partKeys[i]] = settings->value(partKeys[i] + "/" + "used").toString().toInt();
                    ui->tableWidget->item(i, 0)->setText(settings->value(partKeys[i] + "/" + "limit").toString()  +
                                                         QString::fromLocal8Bit("次"));
                    ui->tableWidget->item(i, 1)->setText(settings->value(partKeys[i] + "/" + "used").toString() +
                                                         QString::fromLocal8Bit("次"));
                }
                else
                {
                    QStringList limitTime = settings->value(partKeys[i] + "/" + "limit").
                                            toString().split("@").first().split("-");
                    QDate dateLast(limitTime.at(0).toInt(), limitTime.at(1).toInt(), limitTime.at(2).toInt());
                    QStringList curTime = QDateTime::currentDateTime().toString("yyyy-MM-dd").split("-");
                    QDate dateNow(curTime.at(0).toInt(), curTime.at(1).toInt(), curTime.at(2).toInt());
                    int timeRemaining = settings->value(partKeys[i] + "/" + "limit").
                                        toString().split("@").last().toInt();
                    ui->tableWidget->item(i, 0)->setText(QString::number(timeRemaining) + QString::fromLocal8Bit("天"));
                    ui->tableWidget->item(i, 1)->setText(QString::number(timeRemaining - dateLast.daysTo(dateNow)) +
                                                         QString::fromLocal8Bit("天"));
                }
            }
        }
        delete settings;
        settings = nullptr;
    }
}

int WearingPartsListUser::GetUseNumber(QString key, QString model)
{
    UpdateList(model);
    QList<QString> list = m_useNumber.keys();
    for (int i = 0; i < list.size(); i++)
    {
        if (list[i].contains(key))
            return m_useNumber.value(list[i]);
    }
    return 0;
}

void WearingPartsListUser::Init()
{
#ifdef BAOLONGDEVICE
    setWindowIcon(QIcon("://MyStyle/icon/favicon.ico"));
#else
    setWindowIcon(QIcon("://MyStyle/icon/tool.ico"));
#endif
    ui->tableWidget->setShowGrid(true);
    ui->tableWidget->verticalHeader()->setVisible(true);
    ui->tableWidget->horizontalHeader()->setVisible(true);
    ui->tableWidget->verticalHeader()->setDefaultSectionSize(35);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);  //自适应列宽
    ui->tableWidget->setStyleSheet("QTableWidget::item{border:1px solid black;}");
    ui->tableWidget->verticalHeader()->setStyleSheet("QHeaderView::section{background:skyblue;}"); //设置表头的背景色
    ui->tableWidget->horizontalHeader()->setStyleSheet("QHeaderView::section{background:skyblue;}"); //设置表头的背景色
    //    QPalette pal;
    //    pal.setColor(QPalette::Base, QColor(65, 255, 127));
    //    pal.setColor(QPalette::AlternateBase, QColor(85, 255, 255));
    //    ui->tableWidget->setPalette(pal);
    //    ui->tableWidget->setAlternatingRowColors(true);
    InitList("");
}

void WearingPartsListUser::InitList(QString model)
{
    QDateTime currentTimeFile = QDateTime::currentDateTime();
    QString timeStringFile = currentTimeFile.toString("yyyy-MM-dd");
    QSettings *settings = new QSettings(m_consumables, QSettings::NativeFormat);
    QStringList partKeys = settings->allKeys();
    //删除多余的键
    for (int i = 0; i < partKeys.size(); i++)
    {
        partKeys[i] = partKeys[i].split("/").first();
    }
    for (int i = 0; i < partKeys.count(); i++)
    {
        for (int k = i + 1; k <  partKeys.count(); k++)
        {
            if ( partKeys.at(i) ==  partKeys.at(k))
            {
                partKeys.removeAt(k);
                k--;
            }
        }
    }
    for (int i = 0; i < partKeys.count(); i++)
    {
        if (!partKeys[i].split("@").first().contains(model) &&
            !partKeys[i].split("@").first().contains(QString::fromLocal8Bit("公共参数")) &&
			!partKeys[i].split("@").first().contains(QString::fromLocal8Bit("non")))
        {
            partKeys.removeAt(i);
			i--;
        }
    }
    //生成表格    
	ui->tableWidget->setColumnCount(2);
    ui->tableWidget->setRowCount(partKeys.size());
    ui->tableWidget->horizontalHeader()->resizeSection(0, 100);
    ui->tableWidget->horizontalHeader()->resizeSection(1, 150);
    //先删除以前的数据
    {
        for (int i = 0; i < ui->tableWidget->rowCount(); i++)
        {
            for (int j = 0; j < ui->tableWidget->columnCount(); j++)
            {
                QTableWidgetItem *item = ui->tableWidget->item(i, j);
                if (item)
                {
                    delete item;
                    item = nullptr;
                }
            }
        }
        ui->tableWidget->clear();
    }
    for (int i = 0; i < ui->tableWidget->rowCount(); i++)
    {
        for (int j = 0; j < ui->tableWidget->columnCount(); j++)
        {
            QTableWidgetItem *item = new QTableWidgetItem();
            ui->tableWidget->setItem(i, j, item);
        }
    }
    QStringList horizontalHeaderLabels, verticalHeaderLabels;
    horizontalHeaderLabels << QString::fromLocal8Bit("限制次数/时间") << QString::fromLocal8Bit("已使用次数/剩余检修时间");
    for (int i = 0; i < partKeys.size(); i++)
    {
        if (2 == partKeys[i].split("@").size())
        {
            if (LIMITFREQUENCY == partKeys[i].split("@").last().toInt())
            {
                QString text = partKeys[i].split("@").first();
                if (text.contains(model))
                    text.remove("-" + model);
                verticalHeaderLabels << text;
                ui->tableWidget->item(i, 0)->setText(settings->value(partKeys[i] + "/" + "limit").toString()  +
                                                     QString::fromLocal8Bit("次"));
                ui->tableWidget->item(i, 1)->setText(settings->value(partKeys[i] + "/" + "used").toString() +
                                                     QString::fromLocal8Bit("次"));
                m_useNumber[partKeys[i]] = settings->value(partKeys[i] + "/" + "used").toString().toInt();
            }
            else
            {
                QStringList limitTime = settings->value(partKeys[i] + "/" + "limit").
                                        toString().split("@").first().split("-");
                QDate dateLast(limitTime.at(0).toInt(), limitTime.at(1).toInt(), limitTime.at(2).toInt());
                QStringList curTime = QDateTime::currentDateTime().toString("yyyy-MM-dd").split("-");
                QDate dateNow(curTime.at(0).toInt(), curTime.at(1).toInt(), curTime.at(2).toInt());
                int timeRemaining = settings->value(partKeys[i] + "/" + "limit").
                                    toString().split("@").last().toInt();
                QString text = partKeys[i].split("@").first();
                if (text.contains(model))
                    text.remove("-" + model);
                verticalHeaderLabels << text;
                ui->tableWidget->item(i, 0)->setText(QString::number(timeRemaining) + QString::fromLocal8Bit("天"));
                ui->tableWidget->item(i, 1)->setText(QString::number(timeRemaining - dateLast.daysTo(dateNow)) +
                                                     QString::fromLocal8Bit("天"));
            }
        }
        ui->tableWidget->item(i, 0)->setFont(QFont("song", 14));
        ui->tableWidget->item(i, 1)->setFont(QFont("song", 14));
        ui->tableWidget->item(i, 0)->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        ui->tableWidget->item(i, 1)->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    }
    delete settings;
    settings = nullptr;
    ui->tableWidget->setHorizontalHeaderLabels(horizontalHeaderLabels);
    ui->tableWidget->setVerticalHeaderLabels(verticalHeaderLabels);
}
