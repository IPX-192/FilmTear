#ifndef PARAMMANAGER_H
#define PARAMMANAGER_H

#include <QObject>
#include <QComboBox>
#include <unordered_map>
#include "../../interface/singleton.h"
#include "../../interface/coreinterface.h"
#include "../../interface/plugininterface.h"
#include "ParamDef.h"
#include "VisAppBus.h"

#define  GlobalParam (ParamManager::instance())

class PARAMMANAGER_EXPORT ParamManager : public QObject
{
    Q_OBJECT
    SINGLETON_DECL(ParamManager)
protected:
    explicit ParamManager(QObject *parent = nullptr);

public:
	bool             LoadPlatformMatrix();  //读取平台映射
    bool             SavePlatformMatrix();
    bool             LoadRecipeProduct();   //读取产品配方
    bool             SaveRecipeProduct();   //读取产品配方

    bool             SaveRecipeList(QString filename,QStringList listValue);   //保存配方列表
    bool             LoadRecipeList(QString filename,QStringList& listValue, QComboBox* combox);  //读取配方列表

public:          
    QVisF_Interface*  frameCore = nullptr;
    bool              emptyRun = true;
    bool              flagOffline = false;
    bool              autoRunning = false;
    RecipeProduct     recipeProduct;
    RecipeMotor       recipeMotor;
    RecipeVision      recipeVison;
    RecipeTray        recipeTray;
    RecipeFilmTear    recipeFilmTear;
    RecipeGrip        recipeGrip;
    RecipeDirty       recipeDirty;
    HardwareParam     hardwareParam;
    SystemParam       systemParam;
signals:

public slots:
};



#endif // PARAMMANAGER_H
