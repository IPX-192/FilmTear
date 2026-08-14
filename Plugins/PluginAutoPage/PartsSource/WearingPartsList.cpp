#include <QSettings>
#include <QDateTime>
#include <QVBoxLayout>
#include <QInputDialog>
#include <QMessageBox>
#include "WearingPartsList.h"
#include "ui_WearingPartsList.h"

WearingPartsList::WearingPartsList(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WearingPartsList)
{
    ui->setupUi(this);
    Init();
}

WearingPartsList::~WearingPartsList()
{
    delete ui;
}

void WearingPartsList::Overhaul(int station, QString model)
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
        for (int k = i + 1; k < partKeys.count(); k++)
        {
            if ( partKeys.at(i) ==  partKeys.at(k))
            {
                partKeys.removeAt(k);
                k--;
            }
        }
    }
    for (int i = 0; i < partKeys.size(); i++)
    {
        if (2 == partKeys[i].split("@").size())
        {
            if (LIMITFREQUENCY == partKeys[i].split("@").last().toInt())
            {
                QString text, textModule;
                if (station != -1)
                    textModule = QString::fromLocal8Bit("%1-工位%2").arg(model).arg(station);
                if (station != -1 && model == "non")
                    text = QString::fromLocal8Bit("non-工位%1").arg(station);
                if ((station == -1 && partKeys[i].contains(QString::fromLocal8Bit("公共参数"))) || (station != -1 && model == "non" && partKeys[i].contains(text)) ||
					(station != -1 && partKeys[i].contains(textModule)))
                {
                    int limitValue = settings->value(partKeys[i] + "/" + "limit").toInt(),
                            usedValue = settings->value(partKeys[i] + "/" + "used").toInt();
                    settings->setValue(partKeys[i] + "/" + "used", ++usedValue);
                    if (m_nameToParts.value(partKeys[i].split("@").first()))
                        m_nameToParts.value(partKeys[i].split("@").first())->SetValue(usedValue, LIMITFREQUENCY);
                    if (usedValue > limitValue)
                    {
                         QMessageBox::warning(nullptr, QString::fromLocal8Bit("警告"), QString::fromLocal8Bit("%1已到使用寿命").arg(partKeys[i].split("@").first()));
                    }
                }
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
                settings->setValue(partKeys[i] + "/" + "used", timeRemaining - dateLast.daysTo(dateNow));
                if (m_nameToParts.value(partKeys[i].split("@").first()))
                    m_nameToParts.value(partKeys[i].split("@").first())->
                            SetValue(timeRemaining - dateLast.daysTo(dateNow), LIMITTIME);
                if (settings->value(partKeys[i] + "/" + "used").toInt() <= 0)
                {
                    QMessageBox::warning(nullptr, QString::fromLocal8Bit("警告"), QString::fromLocal8Bit("请检修：%1").arg(partKeys[i].split("@").first()));
                }
            }
        }
    }
    delete settings;
    settings = nullptr;
}

void WearingPartsList::SetMoudleName(QStringList name)
{
    m_moudleModel = name;
}

void WearingPartsList::closeEvent(QCloseEvent *event)
{
    for (int i = 0; i < ui->tableWidget->rowCount(); i++)
    {
        for (int j = 0; j < ui->tableWidget->columnCount(); j++)
        {
            QWidget *widget = ui->tableWidget->cellWidget(i, j);
            if (widget)
            {
                delete widget;
                widget = nullptr;
            }
        }
    }
    ui->tableWidget->clear();
    m_nameToParts.clear();
}

void WearingPartsList::showEvent(QShowEvent *event)
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
    //生成表格
    ui->tableWidget->setColumnCount(2);
    ui->tableWidget->setRowCount(partKeys.size() / 2 + partKeys.size() % 2);
    WearingParts *part = nullptr;
    for (int i = 0; i < partKeys.size(); i++)
    {
        if (2 == partKeys[i].split("@").size())
        {
            if (LIMITFREQUENCY == partKeys[i].split("@").last().toInt())
            {
                part = new WearingParts(this);
                part->SetWearingPartName(partKeys[i].split("@").first(), LIMITFREQUENCY);
                m_nameToParts[partKeys[i].split("@").first()] = part;
                ui->tableWidget->setCellWidget(i / 2, i % 2, part);
                ui->tableWidget->setColumnWidth(i % 2, 270);
                ui->tableWidget->setRowHeight(i / 2, 175);
                int limitValue = settings->value(partKeys[i] + "/" + "limit").toInt(),
                        usedValue = settings->value(partKeys[i] + "/" + "used").toInt();
                part->SetValue(limitValue, usedValue, LIMITFREQUENCY);
                if (usedValue > limitValue)
                {
                    QMessageBox::warning(nullptr, QString::fromLocal8Bit("警告"), QString::fromLocal8Bit("%1已到使用寿命").arg(partKeys[i].split("@").first()));
                }
                connect(part, &WearingParts::operationLog, [=](QString message){
                    emit operationLog(message);
                });
            }
            else
            {
                part = new WearingParts(this);
                part->SetWearingPartName(partKeys[i].split("@").first(), LIMITTIME);
                m_nameToParts[partKeys[i].split("@").first()] = part;
                ui->tableWidget->setCellWidget(i / 2, i % 2, part);
                ui->tableWidget->setColumnWidth(i % 2, 270);
                ui->tableWidget->setRowHeight(i / 2, 175);
                QStringList limitTime = settings->value(partKeys[i] + "/" + "limit").
                                        toString().split("@").first().split("-");
                QDate dateLast(limitTime.at(0).toInt(), limitTime.at(1).toInt(), limitTime.at(2).toInt());
                QStringList curTime = QDateTime::currentDateTime().toString("yyyy-MM-dd").split("-");
                QDate dateNow(curTime.at(0).toInt(), curTime.at(1).toInt(), curTime.at(2).toInt());
                int timeRemaining = settings->value(partKeys[i] + "/" + "limit").
                                    toString().split("@").last().toInt();
                part->SetValue(timeRemaining, timeRemaining - dateLast.daysTo(dateNow), LIMITTIME);
                settings->setValue(partKeys[i] + "/" + "used", timeRemaining - dateLast.daysTo(dateNow));
                if (settings->value(partKeys[i] + "/" + "used").toInt() <= 0)
                {
                    QMessageBox::warning(nullptr, QString::fromLocal8Bit("警告"), QString::fromLocal8Bit("请检修：%1").arg(partKeys[i].split("@").first()));
                }
                connect(part, &WearingParts::operationLog, [=](QString message){
                    emit operationLog(message);
                });
            }
        }
    }
    delete settings;
    settings = nullptr;
}

void WearingPartsList::Init()
{
#ifdef BAOLONGDEVICE
    setWindowIcon(QIcon("://MyStyle/icon/favicon.ico"));
#else
    setWindowIcon(QIcon("://MyStyle/icon/tool.ico"));
#endif
    ui->tableWidget->verticalHeader()->setVisible(false);
    ui->tableWidget->horizontalHeader()->setVisible(false);
    ui->tableWidget->setStyleSheet("QTableWidget::item{border:1px solid ;"
                                   "background-color: qlineargradient(spread:pad, x1:0.5, y1:0, x2:0.5, y2:1, "
                                   "stop:0 rgba(188, 231, 251, 255), stop:1 rgba(255, 255, 255, 255));}");

    connect(ui->btnAddPart, SIGNAL(clicked()), this, SLOT(AddPart()));
    connect(ui->btnDeletePart, SIGNAL(clicked()), this, SLOT(DeletePart()));
}

void WearingPartsList::AddPart()
{
    QStringList stringList;
    //左右工位
    stringList << QString::fromLocal8Bit("公共参数");
    for (int i = 0; i < 10; i++)
    {
        stringList << QString::fromLocal8Bit("工位%1").arg(i);
    }
    QString station = QInputDialog::getItem(this, QString::fromLocal8Bit("选择工位"),
                                            QString::fromLocal8Bit("工位"), stringList);
    if (station.isEmpty())
    {
        QMessageBox::warning(nullptr, QString::fromLocal8Bit("警告"), QString::fromLocal8Bit("输入为空，不予添加"));
        return ;
    }
    QString moudleName;
    if (station != QString::fromLocal8Bit("公共参数"))
    {
        QStringList list;
        list << QString::fromLocal8Bit("不绑定机种");
        list += m_moudleModel;
        moudleName = QInputDialog::getItem(this, QString::fromLocal8Bit("是否绑定机种"),
                                                   QString::fromLocal8Bit("机种名"), list);
        if (moudleName == QString::fromLocal8Bit("不绑定机种"))
            moudleName = "non";
    }

    stringList.clear();
    stringList << QString::fromLocal8Bit("限制使用次数") << QString::fromLocal8Bit("限制使用时间");
    QString name = QInputDialog::getText(this, QString::fromLocal8Bit("输入名称"),
                                         QString::fromLocal8Bit("名称"));
    if (name.isEmpty())
    {
        //报警，输入名称为空
        QMessageBox::warning(nullptr, QString::fromLocal8Bit("警告"), QString::fromLocal8Bit("输入名称为空，不予添加"));
        return ;
    }
    if (!moudleName.isEmpty())
        name += "-" + moudleName;        
    name = name + "-" + station;
    bool ok = false;
    QString type = QInputDialog::getItem(this, QString::fromLocal8Bit("限制次数或时间"),
                                         QString::fromLocal8Bit("次数或者时间"), stringList, 0, false, &ok);
    if (!ok)
    {
        //报警，输入名称为空
        QMessageBox::information(nullptr, QString::fromLocal8Bit("提示"), QString::fromLocal8Bit("取消操作"));
        return ;
    }
    int limitValue = QInputDialog::getInt(this, QString::fromLocal8Bit("输入限制值"),
                                          QString::fromLocal8Bit("限制的次数或时间天数"));
    QSettings *settings = new QSettings(m_consumables, QSettings::NativeFormat);
    if (QString::fromLocal8Bit("限制使用次数") == type)
    {
        if (settings->value(name + "@" + QString::number(LIMITFREQUENCY) + "/" + "limit").toString().isEmpty())
        {
            settings->setValue(name + "@" + QString::number(LIMITFREQUENCY) + "/" + "limit", limitValue);
            settings->setValue(name + "@" + QString::number(LIMITFREQUENCY) + "/" + "used", 0);
        }
        else
        {
            QMessageBox::warning(nullptr, QString::fromLocal8Bit("警告"), QString::fromLocal8Bit("%1维护/保养件已经存在").arg(name));
            delete settings;
            settings = nullptr;
            return ;
        }
    }
    else
    {
        QDateTime currentTimeFile = QDateTime::currentDateTime();
        QString timeStringFile = currentTimeFile.toString("yyyy-MM-dd");
        if (settings->value(name + "@" + QString::number(LIMITTIME) + "/" + "limit").toString().isEmpty())
        {
            settings->setValue(name + "@" + QString::number(LIMITTIME) + "/" + "limit",
                               timeStringFile + "@" + QString::number(limitValue));
            settings->setValue(name + "@" + QString::number(LIMITTIME) + "/" + "used", limitValue);
        }
        else
        {
            QMessageBox::warning(nullptr, QString::fromLocal8Bit("警告"), QString::fromLocal8Bit("%1维护/保养件已经存在").arg(name));
            delete settings;
            settings = nullptr;
            return ;
        }
    }
    delete settings;
    settings = nullptr;
    close();
    show();
    emit operationLog(QString::fromLocal8Bit("%1,维护/保养件添加").arg(name));
    QMessageBox::information(nullptr, QString::fromLocal8Bit("提示"), QString::fromLocal8Bit("%1维护/保养件添加成功").arg(name));
}

void WearingPartsList::DeletePart()
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
    bool ok = false;
    QString removeName = QInputDialog::getItem(this, QString::fromLocal8Bit("删除维护/保养件"),
                                               QString::fromLocal8Bit("维护/保养件名称"), partKeys, 0, false, &ok);
    if (!ok)
    {
        //报警，输入名称为空
        QMessageBox::information(nullptr, QString::fromLocal8Bit("提示"), QString::fromLocal8Bit("取消操作"));
        return ;
    }
    settings->remove(removeName);
    delete settings;
    settings = nullptr;
    close();
    show();
    emit operationLog(QString::fromLocal8Bit("%1,维护/保养件删除").arg(removeName.split("@").first()));
    QMessageBox::information(nullptr, QString::fromLocal8Bit("提示"), QString::fromLocal8Bit("删除成功"));
}
