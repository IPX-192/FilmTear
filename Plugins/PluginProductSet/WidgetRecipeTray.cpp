#include "WidgetRecipeTray.h"
#include "ui_WidgetRecipeTray.h"
#include <QSettings>
#include <QMessageBox>
#include <QFile>
#include <QCoreApplication>
#include "VisUIParam.h"
#include "Tray/TrayCtrl.h"
#include "VisMotorManager.h"
#include "VisMotorToolData.h"

using namespace VisMotorToolSpace;

WidgetRecipeTray::WidgetRecipeTray(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WidgetRecipeTray)
{
    ui->setupUi(this);
    InitWidget();
}

WidgetRecipeTray::~WidgetRecipeTray()
{
    delete ui;
}

void WidgetRecipeTray::InitWidget()
{
    QStringList listGripName;
    listGripName << u8"壳体夹爪" << u8"PCB夹爪";
    QMap<int,QVector<TrayType>>mapTrayType;
    mapTrayType[0] << TestTray1 ;
    mapTrayType[1] << TestTray2;
    QVector<QStringList>vecGripTrayName;
    vecGripTrayName.resize(2);   //0:上料夹爪  1:下料夹爪
    vecGripTrayName[0] << u8"壳体盘";
    vecGripTrayName[1] << u8"PCB盘";
    QVector<QVector<QVector4D>*>vecTrayToPos;
    vecTrayToPos << &m_recipeTray.feedTrayPosHolder;
    vecTrayToPos << &m_recipeTray.feedTrayPosPCB;
    int indexTray = 0;
    for (int i = 0; i < listGripName.size(); i++) {
        QGridLayout* layout = new QGridLayout();
        layout->setMargin(2);
        layout->setSpacing(0);
        QTabWidget* tabWidget = new QTabWidget(ui->tabWidget);
        for (int j = 0; j < vecGripTrayName[i].size(); j++) {
            TrayCtrl* trayWidget = new TrayCtrl(tabWidget);
            trayWidget->setCtrlIndex(indexTray);
            trayWidget->setTrayName(QString::fromLocal8Bit(""));
            trayWidget->setSilosName(QString::fromLocal8Bit(""));
            QList<int> unUsedList;
            trayWidget->initTrayModel(4, 4, unUsedList);
            trayWidget->initSilosModel(4);
            trayWidget->SetSilosVisible(false);
            trayWidget->RenameCellName();
            trayWidget->setProperty("GripType", i);
            trayWidget->setProperty("TrayType", (int)mapTrayType[i][j]);
            connect(trayWidget, &TrayCtrl::sigTraySelected, this, &WidgetRecipeTray::slotTrayClick);
            tabWidget->addTab(trayWidget, vecGripTrayName[i][j]);
            TrayType trayType = mapTrayType[i][j];
            m_mapTray[(GripType)i][trayType] = trayWidget;
            m_trayToPos[trayWidget] = vecTrayToPos[indexTray++];
        }
        ui->tabWidget->addTab(tabWidget, listGripName[i]);
    }
}

void WidgetRecipeTray::LoadUIParam()
{
    RecipeTray& recipeTray = GlobalParam->recipeTray;
    QString filename = recipeTray.filepath + "Tray.xml";
    GlobalParam->LoadRecipeList(filename, recipeTray.listRecipe, ui->comboBox_Recipe);
    LoadRecipeFile();
}

void WidgetRecipeTray::SaveUIParam()
{
    if (m_recipeTray.filepath.isEmpty())
        m_recipeTray.filepath = GlobalParam->recipeTray.filepath;
    if (m_recipeTray.curRecipe.isEmpty())
        m_recipeTray.curRecipe = GlobalParam->recipeTray.curRecipe;
    QString filename = m_recipeTray.filepath + m_recipeTray.curRecipe + ".ini";
    m_recipeTray.WriteTrayFile(filename, this);
    if (GlobalParam->recipeTray.curRecipe == m_recipeTray.curRecipe){
        //点位特殊数据赋值
        GlobalParam->recipeTray =m_recipeTray;
        //刷新界面上的料盘显示
        VisAppBus::postEvent("TrayUpdate", 0, false);
    }
}

void WidgetRecipeTray::UpdateParamToUI()
{
    RecipeTray& recipeTray = GlobalParam->recipeTray;
    if (m_recipeTray.curRecipe.isEmpty())
        m_recipeTray.curRecipe = recipeTray.curRecipe;
    QString filename = recipeTray.filepath + m_recipeTray.curRecipe + ".ini";
    m_recipeTray.ReadTrayFile(filename, this);
    for (int i = 0; i <= 2; i++)
        InitTrayCell((TrayType)i);
}

int WidgetRecipeTray::LoadRecipeFile()
{
	//curRecipe为空或不在列表时,优先当前产品对应的配方
	QString prefer = GlobalParam->recipeProduct.GetCurProductRecipe("recipeTray");
	if (GlobalParam->recipeTray.curRecipe.isEmpty()
		|| !GlobalParam->recipeTray.listRecipe.contains(GlobalParam->recipeTray.curRecipe)) {
		int idx = GlobalParam->recipeTray.listRecipe.indexOf(prefer);
		if (idx < 0) idx = 0;
		if (idx < GlobalParam->recipeTray.listRecipe.size())
			GlobalParam->recipeTray.curRecipe = GlobalParam->recipeTray.listRecipe.at(idx);
	}
    QString filename = GlobalParam->recipeTray.filepath + GlobalParam->recipeTray.curRecipe + ".ini";
    GlobalParam->recipeTray.ReadTrayFile(filename,this);
    m_recipeTray = GlobalParam->recipeTray;
    ui->comboBox_Recipe->blockSignals(true);
    int index = m_recipeTray.listRecipe.indexOf(m_recipeTray.curRecipe);
    if (index < 0) index = m_recipeTray.listRecipe.indexOf(prefer);   //当前产品对应配方
    if (index < 0) index = 0;
    ui->comboBox_Recipe->setCurrentIndex(index);
    ui->comboBox_Recipe->blockSignals(false);
    for (int i = 0; i <= 2; i++)
        InitTrayCell((TrayType)i);

    //刷新界面上的料盘显示
    VisAppBus::postEvent("TrayUpdate", 0, false);

    return 0;
}

void WidgetRecipeTray::on_btnSave_clicked()
{
    QString recipeName = ui->lineEdit->text();
    if (recipeName.isEmpty()) {
        QMessageBox::information(this, u8"提示信息", u8"保存的配方名不能为空");
        return;
    }
    RecipeTray& recipeTray = GlobalParam->recipeTray;
    if (recipeTray.listRecipe.contains(recipeName)) {
        QMessageBox::information(this, u8"提示信息", u8"保存的配方名不能重复");
        return;
    }
    ui->lineEdit->setText("");
    ui->comboBox_Recipe->blockSignals(true);
    ui->comboBox_Recipe->addItem(recipeName);
    ui->comboBox_Recipe->blockSignals(false);
    recipeTray.listRecipe.push_back(recipeName);

    //判断当前文件是否存在
    QString appPath = QCoreApplication::applicationDirPath();
    QString sDefaultTrayFilePath = appPath + "/Config/Product/recipeTray/EQ4-100.ini";

    QString curFileName;
    if (!recipeTray.curRecipe.isEmpty())
        curFileName = recipeTray.filepath + recipeTray.curRecipe + ".ini";
    else
        curFileName = sDefaultTrayFilePath;

    QFile file(curFileName);
    if (!file.exists()) {
        ShowSystemLog(Log_Error, QString(u8"当前配方文件不存在,自动拷贝默认文件！"));

        if (!QFile::exists(sDefaultTrayFilePath)) {
            QMessageBox::information(this, u8"提示信息", u8"缺少默认托盘配置文件EQ4-100.ini");
            return;
        }
        QFile::copy(sDefaultTrayFilePath, curFileName); //自动生成一次当前文件
        curFileName = sDefaultTrayFilePath;
    }

    QString newFileName = recipeTray.filepath + recipeName + ".ini";
    QFile::copy(curFileName, newFileName);

    QString filename = recipeTray.filepath + "Tray.xml";
    GlobalParam->SaveRecipeList(filename, recipeTray.listRecipe);
}



void WidgetRecipeTray::on_comboBox_Recipe_currentIndexChanged(const QString &arg1)
{
    RecipeTray& recipeTray = GlobalParam->recipeTray;
    recipeTray.curRecipe = arg1;
    QString filename = recipeTray.filepath + arg1 + ".ini";
    bool bRet = recipeTray.ReadTrayFile(filename, this);
    if (false == bRet)
        ShowSystemLog(Log_Error, QString(u8"托盘配方文件加载%1失败！").arg(filename));
    m_recipeTray = recipeTray;
    for (int i = 0; i <= 2; i++)
        InitTrayCell((TrayType)i);
}

void WidgetRecipeTray::InitTrayCell(TrayType type)
{
    if (m_selectTray != nullptr)
        m_selectTray->ClearSelect();
    m_selectTray = nullptr;
    int rows = 0, cols = 0;
    if (type == TestTray1) {
        rows = m_recipeTray.trayRowsHolder;
        cols = m_recipeTray.trayColsHolder;
        m_recipeTray.feedTrayPosHolder.resize(rows* cols);
    }
    else if (type == TestTray2) {
        rows = m_recipeTray.trayRowsPCB;
        cols = m_recipeTray.trayColsPCB;
        m_recipeTray.feedTrayPosPCB.resize(rows * cols);
    }

    for (int i = FeedGrip; i <= BlankGrip; i++) {
        if (!m_mapTray.contains((GripType)i))continue;
        if (!m_mapTray[(GripType)i].contains(type))continue;
        m_mapTray[(GripType)i][type]->initTrayModel(rows, cols);
        m_mapTray[(GripType)i][type]->RenameCellName();
    }
}

void WidgetRecipeTray::on_spinBox_trayRowsHolder_editingFinished()
{
    int value = ui->spinBox_trayRowsHolder->value();
    if (value != m_recipeTray.trayRowsHolder) {
        m_recipeTray.trayRowsHolder = ui->spinBox_trayRowsHolder->value();
        InitTrayCell(TestTray1);
    }
}

void WidgetRecipeTray::on_spinBox_trayColsHolder_editingFinished()
{
    int value = ui->spinBox_trayColsHolder->value();
    if (value != m_recipeTray.trayColsHolder) {
        m_recipeTray.trayColsHolder = ui->spinBox_trayColsHolder->value();
        InitTrayCell(TestTray1);
    }
}


void WidgetRecipeTray::on_spinBox_trayRowsPCB_editingFinished()
{
    int value = ui->spinBox_trayRowsPCB->value();
    if (value != m_recipeTray.trayRowsPCB) {
        m_recipeTray.trayRowsPCB = ui->spinBox_trayRowsPCB->value();
        InitTrayCell(TestTray2);
    }
}

void WidgetRecipeTray::on_spinBox_trayColsPCB_editingFinished()
{
    int value = ui->spinBox_trayColsPCB->value();
    if (value != m_recipeTray.trayColsPCB) {
        m_recipeTray.trayColsPCB = ui->spinBox_trayColsPCB->value();
        InitTrayCell(TestTray2);
    }
}

void WidgetRecipeTray::ShowCurHoleInfo(GripType gripType, TrayType trayType, int index)
{
    m_curHole = index;
    QVector4D movePos = m_trayToPos[m_selectTray]->at(m_curHole);
    ui->doubleSpinBoxPosX->blockSignals(true);
    ui->doubleSpinBoxPosY->blockSignals(true);
    ui->doubleSpinBoxPosZ->blockSignals(true);
    ui->doubleSpinBoxPosR->blockSignals(true);
    ui->doubleSpinBoxPosX->setValue(movePos.x());
    ui->doubleSpinBoxPosY->setValue(movePos.y());
    ui->doubleSpinBoxPosZ->setValue(movePos.z());
    ui->doubleSpinBoxPosR->setValue(movePos.w());
    ui->doubleSpinBoxPosX->blockSignals(false);
    ui->doubleSpinBoxPosY->blockSignals(false);
    ui->doubleSpinBoxPosZ->blockSignals(false);
    ui->doubleSpinBoxPosR->blockSignals(false);
    if (ui->checkBoxMove->isChecked()) {
        QMap<QString, double> moveGroup;
        moveGroup = VisMotorDataInstance->GetPosMap(HolderGripSafe);
        moveGroup.unite(VisMotorDataInstance->GetPosMap(PCBGripSafe));
        int nRes = VisMotorInstance->MoveAbsGroup(moveGroup);
        if (nRes != 0)return;
        moveGroup.clear();
        moveGroup[trayType==TestTray1? MotorHolderGantryX: MotorPCBGantryX] = movePos.x();
        moveGroup[trayType == TestTray1 ? MotorHolderGantryY : MotorPCBGantryY] = movePos.y();
        moveGroup[trayType == TestTray1 ? MotorHolderGripR : MotorPCBGripR] = movePos.w();

        nRes = VisMotorInstance->MoveAbsGroup(moveGroup);
        if (nRes != 0)return;
        if (!ui->checkBoxMoveZ->isChecked())return;
        moveGroup.clear();
        moveGroup[trayType == TestTray1 ? MotorHolderGantryZ : MotorPCBGantryZ] = movePos.z();
        VisMotorInstance->MoveAbsGroup(moveGroup);
    }
}

void WidgetRecipeTray::on_btnLoadPosToHole_clicked()
{
    if (m_selectTray == nullptr)return;
    int gripType = m_selectTray->property("GripType").toInt();
    int trayType = m_selectTray->property("TrayType").toInt();
    double posX, posY, posZ, posR;
    VisMotorInstance->GetCurPos(trayType == TestTray1 ? MotorHolderGantryX : MotorPCBGantryX, posX);
    VisMotorInstance->GetCurPos(trayType == TestTray1 ? MotorHolderGantryY : MotorPCBGantryY, posY);
    VisMotorInstance->GetCurPos(trayType == TestTray1 ? MotorHolderGantryZ : MotorPCBGantryZ, posZ);
    VisMotorInstance->GetCurPos(trayType == TestTray1 ? MotorHolderGripR : MotorPCBGripR, posR);

    (*m_trayToPos[m_selectTray])[m_curHole]= QVector4D(posX, posY, posZ, posR);
    ui->doubleSpinBoxPosX->blockSignals(true);
    ui->doubleSpinBoxPosY->blockSignals(true);
    ui->doubleSpinBoxPosZ->blockSignals(true);
    ui->doubleSpinBoxPosR->blockSignals(true);
    ui->doubleSpinBoxPosX->setValue(posX);
    ui->doubleSpinBoxPosY->setValue(posY);
    ui->doubleSpinBoxPosZ->setValue(posZ);
    ui->doubleSpinBoxPosR->setValue(posR);
    ui->doubleSpinBoxPosX->blockSignals(false);
    ui->doubleSpinBoxPosY->blockSignals(false);
    ui->doubleSpinBoxPosZ->blockSignals(false);
    ui->doubleSpinBoxPosR->blockSignals(false);
}

void WidgetRecipeTray::on_btnCalcuTrayPos_clicked()
{
    TrayCtrl* curTray = nullptr;
    for (auto it = m_mapTray.begin(); it != m_mapTray.end(); ++it) {
        for (auto iter = it.value().begin(); iter != it.value().end(); ++iter) {
            if (iter.value()->isVisible()) {
                curTray = iter.value();
                break;
            }
        }
    }
    if (curTray == nullptr)return;
    int rows, cols;
    curTray->GetTrayRowCol(rows, cols);
    QVector<QVector4D>* curVecPos = m_trayToPos[curTray];
    if(curVecPos->size()==1)return;
    QVector4D posA = curVecPos->at(0);
    QString curProductName = GlobalParam->recipeProduct.curProduct;
    if (curProductName.contains("EQ4")) {
        //新逻辑:第一行所有点实测(X方向间距不均匀),行间等距,用第一行第一个+最后一排第一个算出所有行
        QVector4D posC = curVecPos->at((rows - 1) * cols);   //最后一排第一个
        double rowSpanX = 0, rowSpanY = 0, rowSpanZ = 0;
        if (rows > 1) {
            rowSpanX = (posC.x() - posA.x()) / (rows - 1);
            rowSpanY = (posC.y() - posA.y()) / (rows - 1);
            rowSpanZ = (posC.z() - posA.z()) / (rows - 1);
        }
        for (int j = 1; j < rows; j++) {
            for (int i = 0; i < cols; i++) {
                QVector4D firstRow = curVecPos->at(i);   //第一行第i列实测点
                (*curVecPos)[j * cols + i].setX(firstRow.x() + j * rowSpanX);
                (*curVecPos)[j * cols + i].setY(firstRow.y() + j * rowSpanY);
                (*curVecPos)[j * cols + i].setZ(firstRow.z() + j * rowSpanZ);
                (*curVecPos)[j * cols + i].setW(posA.w());
            }
        }
    }
    else {
        //旧逻辑:三点插值算出整个料盘
        QVector4D posB = curVecPos->at(cols - 1);
        QVector4D posC = curVecPos->at((rows - 1) * cols);

        double colSpanX= 0,colSpanY=0,colSpanZ=0;
        if(cols>1){
            colSpanX= (posB.x() - posA.x()) / (cols - 1);
            colSpanY = (posB.y() - posA.y()) / (cols - 1);
            colSpanZ = (posB.z() - posA.z()) / (cols - 1);
        }
        double rowSpanX= 0,rowSpanY=0,rowSpanZ=0;
        if(rows>1){
            rowSpanX = (posC.x() - posA.x()) / (rows - 1);
            rowSpanY = (posC.y() - posA.y()) / (rows - 1);
            rowSpanZ = (posC.z() - posA.z()) / (rows - 1);
        }

        for (int j = 0; j < rows; j++) {
            double startX = posA.x() + j * rowSpanX;
            double startY = posA.y() + j * rowSpanY;
            double startZ = posA.z() + j * rowSpanZ;
            for (int i = 0; i < cols; i++) {
                (*curVecPos)[j * cols + i].setX(startX + i * colSpanX);
                (*curVecPos)[j * cols + i].setY(startY + i * colSpanY);
                (*curVecPos)[j * cols + i].setZ(startZ + i * colSpanZ);
                (*curVecPos)[j * cols + i].setW(posA.w());
            }
        }
    }
}

void WidgetRecipeTray::slotTrayClick(int index, int silos, int row, int col)
{
    TrayCtrl* pSender = (TrayCtrl*)sender();
    if (m_selectTray != nullptr && pSender != m_selectTray) {
        m_selectTray->ClearSelect();
    }
    m_selectTray = pSender;
    int trayRows, trayCols;
    m_selectTray->GetTrayRowCol(trayRows, trayCols);
    int gripType = m_selectTray->property("GripType").toInt();
    int trayType = m_selectTray->property("TrayType").toInt();
    ShowCurHoleInfo((GripType)gripType, (TrayType)trayType, row * trayCols + col);
}

void WidgetRecipeTray::on_doubleSpinBoxPosX_valueChanged(double arg1)
{
    if (m_selectTray == nullptr)return;
    (*m_trayToPos[m_selectTray])[m_curHole] = QVector4D(ui->doubleSpinBoxPosX->value(), ui->doubleSpinBoxPosY->value(),
                                                       ui->doubleSpinBoxPosZ->value(), ui->doubleSpinBoxPosR->value());
}

void WidgetRecipeTray::on_doubleSpinBoxPosY_valueChanged(double arg1)
{
    if (m_selectTray == nullptr)return;
    (*m_trayToPos[m_selectTray])[m_curHole] = QVector4D(ui->doubleSpinBoxPosX->value(), ui->doubleSpinBoxPosY->value(),
                                                        ui->doubleSpinBoxPosZ->value(), ui->doubleSpinBoxPosR->value());
}

void WidgetRecipeTray::on_doubleSpinBoxPosZ_valueChanged(double arg1)
{
    if (m_selectTray == nullptr)return;
    (*m_trayToPos[m_selectTray])[m_curHole] = QVector4D(ui->doubleSpinBoxPosX->value(), ui->doubleSpinBoxPosY->value(),
                                                        ui->doubleSpinBoxPosZ->value(), ui->doubleSpinBoxPosR->value());
}

void WidgetRecipeTray::on_doubleSpinBoxPosR_valueChanged(double arg1)
{
    if (m_selectTray == nullptr)return;
    (*m_trayToPos[m_selectTray])[m_curHole] = QVector4D(ui->doubleSpinBoxPosX->value(), ui->doubleSpinBoxPosY->value(),
                                                        ui->doubleSpinBoxPosZ->value(), ui->doubleSpinBoxPosR->value());
}
