#ifndef WEARINGPARTSLISTUSER_H
#define WEARINGPARTSLISTUSER_H

#include <QMap>
#include <QWidget>
#include "WearingParts.h"

namespace Ui {
class WearingPartsListUser;
}

class WearingPartsListUser : public QWidget
{
    Q_OBJECT

public:
    explicit WearingPartsListUser(QWidget *parent = nullptr);
    ~WearingPartsListUser();

    /**
      *@brief 更新易损件列表
      */
    void UpdateList(QString model, bool updateUi = false);

    /**
      *@brief 获取对应key的使用次数，不用全名
      */
    int GetUseNumber(QString key, QString model);

private:
    /**
      *@brief 初始化化函数
      */
    void Init();

    /**
      *@brief 初始化显示列表
      */
    void InitList(QString model);

private:
    Ui::WearingPartsListUser *ui;
    QString m_consumables = "HKEY_CURRENT_USER\\Software\\ViSensing\\Consumables";
    QMap<QString, int> m_useNumber;
};

#endif // WEARINGPARTSLISTUSER_H
