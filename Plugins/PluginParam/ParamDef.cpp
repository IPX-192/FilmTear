#include "ParamDef.h"
#include <QVector4D>
#include <QMetaObject>
#include <QMetaProperty>
#include "VisUIParam.h"

SystemParam& SystemParam::operator=(const SystemParam&other)
{
	QObject* src = (QObject*) & other;
	VisUIParam::QObjectCopy(src, this);
    this->shieldParam=other.shieldParam;
	return *this;
}

RecipeTray &RecipeTray::operator=(const RecipeTray &other)
{
    QObject* src = (QObject*) & other;
    VisUIParam::QObjectCopy(src, this);

    this->feedTrayPosHolder = other.feedTrayPosHolder;
    this->feedTrayPosPCB = other.feedTrayPosPCB;

    return *this;
}

bool RecipeTray::ReadTrayFile(QString filename,QObject* uiObj)
{
	QSettings settings(filename, QSettings::IniFormat);
	settings.beginGroup("Param");
	VisUIParam::LoadIniToUI(&settings, uiObj, this);
	settings.endGroup();
    feedTrayPosHolder.resize(trayRowsHolder * trayColsHolder);
    for (int j = 0; j < feedTrayPosHolder.size(); j++) {
        QStringList listValue = settings.value(QString("HolderTray%1/Pos_%2").arg(1).arg(j + 1)).toString().split(" ");
        if (listValue.size() != 4)return false;
        feedTrayPosHolder[j] = QVector4D(listValue[0].toDouble(), listValue[1].toDouble(), listValue[2].toDouble(), listValue[3].toDouble());
    }
	feedTrayPosPCB.resize(trayRowsPCB * trayColsPCB);
	for (int j = 0; j < feedTrayPosPCB.size(); j++) {
		QStringList listValue = settings.value(QString("PCBTray%1/Pos_%2").arg(1).arg(j + 1)).toString().split(" ");
		if (listValue.size() != 4)return false;
        feedTrayPosPCB[j] = QVector4D(listValue[0].toDouble(), listValue[1].toDouble(), listValue[2].toDouble(), listValue[3].toDouble());
	}

	return true;
}

void RecipeTray::WriteTrayFile(QString filename,QObject* uiObj)
{
	QSettings settings(filename, QSettings::IniFormat);
	settings.beginGroup("Param");
	VisUIParam::SaveUIToIni(&settings, uiObj, this);
	settings.endGroup();
    for (int j = 0; j < feedTrayPosHolder.size(); j++) {
        QString strValue = QString("%1 %2 %3 %4").arg(feedTrayPosHolder[j].x()).arg(feedTrayPosHolder[j].y()).arg(feedTrayPosHolder[j].z()).arg(feedTrayPosHolder[j].w());
        settings.setValue(QString("HolderTray%1/Pos_%2").arg(1).arg(j + 1), strValue);
    }
	for (int j = 0; j < feedTrayPosPCB.size(); j++) {
		QString strValue = QString("%1 %2 %3 %4").arg(feedTrayPosPCB[j].x()).arg(feedTrayPosPCB[j].y()).arg(feedTrayPosPCB[j].z()).arg(feedTrayPosPCB[j].w());
		settings.setValue(QString("PCBTray%1/Pos_%2").arg(1).arg(j + 1), strValue);
	}
}

RecipeVision &RecipeVision::operator=(const RecipeVision &other)
{
    QObject* src = (QObject*) & other;
    VisUIParam::QObjectCopy(src, this);
    return *this;
}

RecipeFilmTear &RecipeFilmTear::operator=(const RecipeFilmTear &other)
{
    QObject* src = (QObject*) & other;
    VisUIParam::QObjectCopy(src, this);
    return *this;
}

void RecipeProduct::UpdateRecipe()
{
    if(curMatrix==nullptr)return;
    for(int i=0;i<vecRecipeDetail.size();i++){
        QVector<QPair<QString,QString>>&groupRecipe=vecRecipeDetail[i];
        QString productname=groupRecipe.at(0).second;
        if (productname == curMatrix->productName) {
            for(int j=0;j<vecRecipeNode.size();j++){
                IRecipeBase*recipeBase=vecRecipeNode[j].second;
                recipeBase->curRecipe=groupRecipe[j].second;
            }
        }
    }
}

