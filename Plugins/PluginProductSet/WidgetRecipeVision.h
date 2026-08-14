#ifndef WIDGETRECIPEVISION_H
#define WIDGETRECIPEVISION_H

#include <QWidget>
#include <opencv2/core.hpp>
#include "ParamManager.h"

namespace Ui {
class WidgetRecipeVision;
}

class WidgetRecipeVision : public QWidget
{
    Q_OBJECT

public:
    explicit WidgetRecipeVision(QWidget *parent = nullptr);
    ~WidgetRecipeVision();

public slots:
    void  LoadUIParam();
    void  SaveUIParam();
    void  UpdateParamToUI();
    int   LoadRecipeFile();     //加载配方详细参数

protected:


private slots:
    void on_btnSave_clicked();
    void on_comboBox_Recipe_currentIndexChanged(const QString &arg1);
    int event_GetCvMatCam(QString AlgorithmName, QString cameraName, cv::Mat &img);
    int event_SetCamEx(QString AlgorithmName, QString cameraName, int CamEx);
    int event_SetLight(QString AlgorithmName, QString LightName, bool bSwitch, int red, int green, int blue, int Lum);

private:
    QStringList ReadCameraNameList();
    QStringList ReadLightNameList();
    void LoadLightChannelMap();
    QWidget* CreateDebugPanel();

public:
    RecipeVision   m_recipeVison;
    bool           m_algorithmInited = false;
    QMap<QString, int> m_lightChannelMap;

private:
    Ui::WidgetRecipeVision *ui;
};

#endif // WIDGETRECIPEVISION_H
