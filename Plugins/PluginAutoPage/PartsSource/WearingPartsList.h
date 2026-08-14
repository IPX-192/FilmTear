#ifndef WEARINGPARTSLIST_H
#define WEARINGPARTSLIST_H

#include <QMap>
#include <QWidget>
#include "WearingParts.h"

namespace Ui {
class WearingPartsList;
}

class WearingPartsList : public QWidget
{
    Q_OBJECT

public:
    explicit WearingPartsList(QWidget *parent = nullptr);
    ~WearingPartsList();

    /**
      *@brief 判断是否需要进行检修,使用次数的易损件调用此函数后使用次数会自动加1
      *@params 0 左工位 1 右工位 -1 通用参数 -2 工位不绑定机种
      */
    void Overhaul(int station = -1, QString model = "");

    //设置机种名
    void SetMoudleName(QStringList name);

protected:
    void closeEvent(QCloseEvent *event);
    void showEvent(QShowEvent *event);

private:
    /**
      *@brief 初始化化函数
      */
    void Init();

    /**
      *@brief 读取注册表的数据，显示易损件列表
      */
    void ReadRegeditData();

private slots:
    /**
      *@brief 增加易损件部件
      */
    void AddPart();

    void DeletePart();

private:
    QString m_consumables = "HKEY_CURRENT_USER\\Software\\ViSensing\\Consumables";
    Ui::WearingPartsList *ui;
    WearingParts m_wearingParts;
    QMap<QString, WearingParts *> m_nameToParts; //名称对应的易损部件
    QStringList m_moudleModel;

signals:
    void operationLog(QString message);
};

#endif // WEARINGPARTSLIST_H
