#ifndef WEARINGPARTS_H
#define WEARINGPARTS_H

#include <QWidget>

namespace Ui {
class WearingParts;
}

enum WearingPartType{LIMITFREQUENCY, LIMITTIME};

class WearingParts : public QWidget
{
    Q_OBJECT

public:
    explicit WearingParts(QWidget *parent = nullptr);
    ~WearingParts();

    /**
      *@brief 设置易损件名称
      *@param name 名称
      *@param type 使用的易损件类型
      */
    void SetWearingPartName(QString &name, WearingPartType type);

    /**
      *@brief 设置限定值及使用值
      *@param limitValue 限定值
      *@param usedValue 使用值
      */
    void SetValue(int limitValue, int usedValue, WearingPartType type);

    /**
      *@brief 设置使用值
      *@param usedValue 使用值
      */
    void SetValue(int usedValue, WearingPartType type);

private:
    void Init();

private slots:
    void SetFreqLimitValue();
    void ResetFreqUsedValue();
    void SetTimeLimitValue();
    void ResetTimeUsedValue();

private:
    Ui::WearingParts *ui;
    QString m_consumables = "HKEY_CURRENT_USER\\Software\\ViSensing\\Consumables";

signals:
    void operationLog(QString message);
};

#endif // WEARINGPARTS_H
