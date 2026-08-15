#include "ParamManager.h"
#include <QCoreApplication>
#include <QSettings>
#include <QTextCodec>
#include <QDir>
#include <QXmlStreamReader>
#include <QDomDocument>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>

#define REGISTER_RECIPE_MODULE(CN_NAME,EN_NAME,CLASS) \
   CLASS.setObjectName(EN_NAME);\
   recipeProduct.vecRecipeNode.push_back(QPair<QString, IRecipeBase*>(CN_NAME, &CLASS));
   

SINGLETON_IMPL(ParamManager)
ParamManager::ParamManager(QObject *parent) : QObject(parent)
{
	REGISTER_RECIPE_MODULE(u8"型号名称", "recipeProduct", recipeProduct);
	REGISTER_RECIPE_MODULE(u8"伺服配方", "recipeMotor", recipeMotor);
	REGISTER_RECIPE_MODULE(u8"视觉配方", "recipeVison", recipeVison);
	REGISTER_RECIPE_MODULE(u8"托盘配方", "recipeTray", recipeTray);
	REGISTER_RECIPE_MODULE(u8"撕膜配方", "recipeFilmTear",recipeFilmTear);
	REGISTER_RECIPE_MODULE(u8"夹爪配方", "recipeGrip", recipeGrip);
	REGISTER_RECIPE_MODULE(u8"脏污配方", "recipeDirty", recipeDirty);
	REGISTER_RECIPE_MODULE(u8"扫码配方", "recipeScanCode", recipeScanCode);
}

bool ParamManager::LoadPlatformMatrix()
{
	recipeProduct.listPlatformMatrix.clear();
	QString fileName = GlobalParam->recipeProduct.productPath + "Matrix.xml";
	QFile inFile(fileName);
	if (!inFile.exists()) return true;
	inFile.open(QIODevice::ReadOnly);
	QString outdata = inFile.readAll();
	inFile.close();

	QDomDocument document;
	QString error;
	int row = 0, column = 0;
	if (!document.setContent(outdata, false, &error, &row, &column)) return false;
	if (document.isNull())return false;
	QDomElement root = document.documentElement();
	if (root.tagName() != "Matrix")return false;

	QDomNodeList matrixList = root.elementsByTagName("platform");
	for (int i = 0; i < matrixList.count(); i++) {
		QDomElement matrixNode = matrixList.at(i).toElement();
		MatrixSetting matrix;
		matrix.platformName = matrixNode.attribute("platformName");
		matrix.productName = matrixNode.attribute("productName");
        matrix.productShow = matrixNode.attribute("productShow");
        matrix.productNo = matrixNode.attribute("productNo");

        matrix.barCodeLimit1 = matrixNode.attribute("barCodeLimit1");
        matrix.barCodeStartPos1 = matrixNode.attribute("barCodeStartPos1");
        matrix.barCodeLimit2 = matrixNode.attribute("barCodeLimit2");
        matrix.barCodeStartPos2 = matrixNode.attribute("barCodeStartPos2");

        matrix.productFamily1 = matrixNode.attribute("productFamily1");
        matrix.fixtureSn1 = matrixNode.attribute("fixtureSn1");
        matrix.productFamily2 = matrixNode.attribute("productFamily2");
        matrix.fixtureSn2 = matrixNode.attribute("fixtureSn2");

        matrix.lensProductNo = matrixNode.attribute("lensProductNo");
        matrix.lensCodeEnable = matrixNode.attribute("lensCodeEnable");
		recipeProduct.listPlatformMatrix.push_back(matrix);
	}

	return true;
}

bool ParamManager::SavePlatformMatrix()
{
	QString fileName = GlobalParam->recipeProduct.productPath + "Matrix.xml";
	QFileInfo fileInfo(fileName);
	fileInfo.dir().mkpath(fileInfo.absolutePath());
	QDomDocument domDocument;
	QString strHeader("version=\"1.0\" encoding=\"UTF-8\"");
	domDocument.appendChild(domDocument.createProcessingInstruction("xml", strHeader));
	QDomElement root = domDocument.createElement("Matrix");
	domDocument.appendChild(root);

	for (int i = 0; i < recipeProduct.listPlatformMatrix.size(); i++) {
		MatrixSetting& matrix = recipeProduct.listPlatformMatrix[i];
		QDomElement matrixNode = domDocument.createElement("platform");
		matrixNode.setAttribute("platformName", matrix.platformName);
		matrixNode.setAttribute("productName", matrix.productName);
        matrixNode.setAttribute("productShow", matrix.productShow);
        matrixNode.setAttribute("productNo", matrix.productNo);

        matrixNode.setAttribute("barCodeLimit1", matrix.barCodeLimit1);
        matrixNode.setAttribute("barCodeStartPos1", matrix.barCodeStartPos1);
        matrixNode.setAttribute("barCodeLimit2", matrix.barCodeLimit2);
        matrixNode.setAttribute("barCodeStartPos2", matrix.barCodeStartPos2);

        matrixNode.setAttribute("productFamily1", matrix.productFamily1);
        matrixNode.setAttribute("fixtureSn1", matrix.fixtureSn1);
        matrixNode.setAttribute("productFamily2", matrix.productFamily2);
        matrixNode.setAttribute("fixtureSn2", matrix.fixtureSn2);

        matrixNode.setAttribute("lensProductNo", matrix.lensProductNo);
        matrixNode.setAttribute("lensCodeEnable", matrix.lensCodeEnable);

		root.appendChild(matrixNode);
	}
	QFile inFile(fileName);
	inFile.open(QIODevice::WriteOnly);
	QTextStream stream(&inFile);
	stream << domDocument.toString();
	inFile.close();

	return true;
}

bool ParamManager::LoadRecipeProduct()
{
    recipeProduct.vecRecipeDetail.clear();
	QString fileName = GlobalParam->recipeProduct.productPath + "Recipe.xml";
	QFile inFile(fileName);
	if (!inFile.exists()) return true;
	inFile.open(QIODevice::ReadOnly);
	QString outdata = inFile.readAll();
	inFile.close();
	
	QDomDocument document;
	QString error;
	int row = 0, column = 0;
	if (!document.setContent(outdata, false, &error, &row, &column)) return false;
	if (document.isNull())return false;
	QDomElement root = document.documentElement();
	if (root.tagName() != "Recipe")return false;
	recipeProduct.curProduct = root.attribute("CurProduct");

	QDomNodeList matrixList = root.elementsByTagName("product");
	for (int i = 0; i < matrixList.count(); i++) {
		QDomElement matrixNode = matrixList.at(i).toElement();
        QVector<QPair<QString,QString>>groupRecipe;
        for(int j=0;j<recipeProduct.vecRecipeNode.size();j++){
            QString objName=recipeProduct.vecRecipeNode[j].second->objectName();
            QString value=matrixNode.attribute(objName);
            groupRecipe.push_back(QPair<QString,QString>(objName,value));
        }
        recipeProduct.vecRecipeDetail.push_back(groupRecipe);
	}

	for (int i = 0; i < recipeProduct.listPlatformMatrix.size(); i++) {
		MatrixSetting&matrix = recipeProduct.listPlatformMatrix[i];
		if (recipeProduct.curProduct == matrix.productShow) {
            recipeProduct.curMatrix=&matrix;
		}
	}
    recipeProduct.UpdateRecipe();

	return true;
}

bool ParamManager::SaveRecipeProduct()
{
	recipeProduct.UpdateRecipe();
	QString fileName = GlobalParam->recipeProduct.productPath + "Recipe.xml";
	QFileInfo fileInfo(fileName);
	fileInfo.dir().mkpath(fileInfo.absolutePath());
	QDomDocument domDocument;
	QString strHeader("version=\"1.0\" encoding=\"UTF-8\"");
	domDocument.appendChild(domDocument.createProcessingInstruction("xml", strHeader));
	QDomElement root = domDocument.createElement("Recipe");
	domDocument.appendChild(root);
	root.setAttribute("CurProduct", recipeProduct.curProduct);
    for (int i = 0; i < recipeProduct.vecRecipeDetail.size(); i++) {
        QDomElement matrixNode = domDocument.createElement("product");
        QVector<QPair<QString,QString>>&groupReceipe=recipeProduct.vecRecipeDetail[i];
        QVector<QPair<QString,IRecipeBase*>> &vecRecipeNode=recipeProduct.vecRecipeNode;
        for(int j=0;j<groupReceipe.size();j++){
             QString nameEn=vecRecipeNode[j].second->objectName();
             matrixNode.setAttribute(nameEn, groupReceipe[j].second);
        }
		
		root.appendChild(matrixNode);
	}

	QFile inFile(fileName);
	inFile.open(QIODevice::WriteOnly | QIODevice::Text);
	QTextStream stream(&inFile);
	stream.setCodec("UTF-8");
	stream << domDocument.toString();
	inFile.close();

	return true;
}


bool ParamManager::SaveRecipeList(QString filename, QStringList listValue)
{
	QFileInfo fileInfo(filename);
	fileInfo.dir().mkpath(fileInfo.absolutePath());
	QDomDocument domDocument;
	QString strHeader("version=\"1.0\" encoding=\"UTF-8\"");
	domDocument.appendChild(domDocument.createProcessingInstruction("xml", strHeader));
	QDomElement root = domDocument.createElement("Recipe");
	domDocument.appendChild(root);

	for (int i = 0; i < listValue.size(); i++) {
		QDomElement itemNode = domDocument.createElement("Item");
		itemNode.setAttribute("name", listValue[i]);
		root.appendChild(itemNode);
	}
	QFile inFile(filename);
	inFile.open(QIODevice::WriteOnly | QIODevice::Text);
	QTextStream stream(&inFile);
	stream.setCodec("UTF-8");
	stream << domDocument.toString();
	inFile.close();

	return true;
}

bool ParamManager::LoadRecipeList(QString filename, QStringList& listValue, QComboBox*combox)
{
	listValue.clear();
	QFile inFile(filename);
	if (!inFile.exists()) return true;
	inFile.open(QIODevice::ReadOnly);
	QString outdata = inFile.readAll();
	inFile.close();

	QDomDocument document;
	QString error;
	int row = 0, column = 0;
	if (!document.setContent(outdata, false, &error, &row, &column)) return false;
	if (document.isNull())return false;
	QDomElement root = document.documentElement();
	if (root.tagName() != "Recipe")return false;

	QDomNodeList itemNodeList = root.elementsByTagName("Item");
	for (int i = 0; i < itemNodeList.count(); i++) {
		QDomElement itemNode = itemNodeList.at(i).toElement();
		listValue<< itemNode.attribute("name");
	}
	combox->blockSignals(true);
	combox->clear();
	for (int i = 0; i < listValue.size(); i++) {
		combox->addItem(listValue[i]);
	}
	combox->blockSignals(false);

	return true;
}
