#pragma execution_character_set("utf-8")
#include "WidgetMesLogin.h"
#include "ui_WidgetMesLogin.h"

WidgetMesLogin::WidgetMesLogin(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WidgetMesLogin)
{
    ui->setupUi(this);

    QString g_appPath = QCoreApplication::applicationDirPath();
    QString filePath = g_appPath + "/url.txt";
    QFile file(filePath);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        if(file.atEnd()) return;
        QString url = file.readLine().trimmed();
        if(file.atEnd()) return;
        QString deviceIp = file.readLine().trimmed();
        if(!url.isEmpty() && !deviceIp.isEmpty())
            MesHttpPost::Instance()->SetMesIpInfo(url, deviceIp);

        this->setWindowTitle(QString("MES:%1").arg(url));
        QString loadStr = file.readLine().trimmed();
        m_load = false;
        QStringList loadList = loadStr.split("=");
        if(loadList.size() == 2){
            if(loadList.at(1) == "true")
                m_load = true;
        }
        file.close();
    }

    // 等待弹窗初始化
    m_waitMessageBox = new QMessageBox(this);
    m_waitMessageBox->setWindowTitle(("提示信息"));
    m_waitMessageBox->setStandardButtons(QMessageBox::NoButton);

    // 初始化缓存数据
    m_allPdLineData.clear();
    m_heartTimer.setInterval(HEARTBEAT_INTERVAL);
    connect(&m_heartTimer, &QTimer::timeout, this, &WidgetMesLogin::SlotPostHeartBeatTimeout);

    ui->lineEdit_user->setText("yeyuan.zuo1");
    ui->lineEdit_passwd->setText("123456");
}

WidgetMesLogin::~WidgetMesLogin()
{
    delete ui;
}

void WidgetMesLogin::closeEvent(QCloseEvent *event)
{
    emit sigCloseEvent();
    m_widgetLoad->close();
}

// 加载全量产品线、产品、工单数据，填充产品线下拉
bool WidgetMesLogin::LoadAllWorkOrderData(QString &errMsg)
{
    errMsg.clear();
    m_waitMessageBox->setText(("获取产品工单信息中,请稍候..."));
    m_waitMessageBox->show();

    //先获取当前可生产的产品和工单信息
    QVector<PdLineList> outData;
    errMsg = MesHttpPost::Instance()->QueryWorkOrderInfo(outData);

    m_waitMessageBox->hide();
    if (!errMsg.isEmpty()) {
        ShowMsg("错误", "获取产品工单信息失败");
        return false;
    }

    // 缓存全局数据
    m_allPdLineData = outData;
    // 清空所有下拉
    ui->comboBox_pdLineList->clear();
    ui->comboBox_pdList->clear();
    ui->comboBox_workOrderList->clear();
    ui->comboBox_workProcess->clear();

    // 填充产品线下拉
    ui->comboBox_pdLineList->blockSignals(true);
    for (const auto& line : m_allPdLineData)
    {
        ui->comboBox_pdLineList->addItem(line.name);    //填充产品线名
    }
    ui->comboBox_pdLineList->blockSignals(false);

    if (!m_allPdLineData.isEmpty())
        on_comboBox_pdLineList_currentIndexChanged(0);

    return true;
}

// 产品线下拉切换
void WidgetMesLogin::on_comboBox_pdLineList_currentIndexChanged(int index)
{
    if (m_allPdLineData.isEmpty() || index < 0 || index >= m_allPdLineData.size())
        return;

    const auto& lineData = m_allPdLineData[index];
    ui->comboBox_pdList->clear();
    ui->comboBox_pdList->blockSignals(true);
    for (const auto& product : lineData.vecPdList)
    {
        ui->comboBox_pdList->addItem(product.name);
    }
    ui->comboBox_pdList->blockSignals(false);
    if (!lineData.vecPdList.isEmpty())
        on_comboBox_pdList_currentIndexChanged(0);
}

// 产品下拉切换
void WidgetMesLogin::on_comboBox_pdList_currentIndexChanged(int index)
{
    int lineIdx = ui->comboBox_pdLineList->currentIndex();
    if (m_allPdLineData.isEmpty() || lineIdx < 0 || index < 0)
        return;

    const auto& lineData = m_allPdLineData[lineIdx];
    if (index >= lineData.vecPdList.size())
        return;

    const auto& prodData = lineData.vecPdList[index];
    ui->comboBox_workOrderList->clear();
    ui->comboBox_workOrderList->blockSignals(true);
    for (const auto& workOrder : prodData.workOrderList.vecWorkOrder)
    {
        ui->comboBox_workOrderList->addItem(workOrder.name, workOrder.workOrderId);
    }
    ui->comboBox_workOrderList->blockSignals(false);
    if (!prodData.workOrderList.vecWorkOrder.isEmpty())
        on_comboBox_workOrderList_currentIndexChanged(0);
}

void WidgetMesLogin::on_comboBox_workOrderList_currentIndexChanged(int index)
{
    ui->comboBox_workProcess->clear();
    QString workOrderId = ui->comboBox_workOrderList->currentData().toString();
    if (workOrderId.isEmpty())
        return;

    // 2. 本地缓存无数据，才调用真实接口
    QString err;
    RefreshProcessCombo(workOrderId, err);
    if (!err.isEmpty())
        ShowMsg("加载工序失败", err);
}

// 登录按钮点击
void WidgetMesLogin::on_pushButton_login_clicked()
{
    if(ui->checkBox_NoMes->isChecked()){
        QString username = ui->lineEdit_user->text();
        sigLogin(username, 0);
        this->hide();
        return;
    }

    SetUiLock(true);
    QString errMsg;
    bool loginOk = DoFullLoginFlow(errMsg);
    SetUiLock(false);

    if (!loginOk)
    {
        ShowMsg(("登录失败"), errMsg);
        return;
    }
    ShowMsg(("提示"), ("登录成功"));

    if(!m_load){
        QString username = ui->lineEdit_user->text();
        sigLogin(username, 0);
        this->hide();
        return;
    }

    if(!m_widgetLoad){
        m_widgetLoad = new WidgetLoad(nullptr);
        m_widgetLoad->setWindowFlags(Qt::Window);
        m_widgetLoad->setWindowTitle("上料界面");
        m_widgetLoad->SetMaterialInfo(m_materialInfo);
        connect(m_widgetLoad, &WidgetLoad::sigLoadDone,
                this, &WidgetMesLogin::onLoadDone);
        connect(m_widgetLoad, &WidgetLoad::sigCloseEvent, [&](){
            this->close();
        });
    }
    m_widgetLoad->show();
    m_widgetLoad->raise();
    m_widgetLoad->activateWindow();

    this->hide();
}

void WidgetMesLogin::SlotPostHeartBeatTimeout()
{
    QString heartBeatMsg =  MesHttpPost::Instance()->StationHeartbeat();
    if(heartBeatMsg.isEmpty())
    {
        return;
    }
    ui->label_heartMsg->setText(heartBeatMsg);
}

void WidgetMesLogin::on_btn_UpdateList_clicked()
{
    QString msgErr;
    LoadAllWorkOrderData(msgErr);
}

void WidgetMesLogin::onLoadDone()
{
    QString username = ui->lineEdit_user->text();
    sigLogin(username, 0);
}

void WidgetMesLogin::ShowMsg(const QString &title, const QString &text)
{
    QMessageBox::information(this, title, text);
}

// 界面锁定解锁
void WidgetMesLogin::SetUiLock(bool lock)
{
    this->setEnabled(!lock);
    this->setCursor(lock ? Qt::WaitCursor : Qt::ArrowCursor);
}

// 根据工单ID刷新工序下拉
bool WidgetMesLogin::RefreshProcessCombo(const QString &workOrderId, QString &errMsg)
{
    errMsg.clear();
    QList<DeviceProcessItem> processList;
    errMsg = MesHttpPost::Instance()->QueryDeviceProcessInfo("",workOrderId, processList);
    if (!errMsg.isEmpty()){
        return false;
    }

    ui->comboBox_workProcess->clear();
    for (const auto& proc : processList)
    {
        //添加工序
        ui->comboBox_workProcess->addItem(proc.processName, proc.processId);
    }
    return true;
}

bool WidgetMesLogin::DoFullLoginFlow(QString &errMsg)
{
    errMsg.clear();
    // 1. 读取界面输入
    QString user = ui->lineEdit_user->text().trimmed();
    QString pwd = ui->lineEdit_passwd->text().trimmed();
    QString workOrderId = ui->comboBox_workOrderList->currentData().toString();
    QString processId = ui->comboBox_workProcess->currentData().toString();

    // 基础输入校验
    if (user.isEmpty() || pwd.isEmpty())
    {
        errMsg = ("用户名/密码不能为空");
        return false;
    }
    if (workOrderId.isEmpty() || processId.isEmpty())
    {
        errMsg = ("请先选择工单和工序");
        return false;
    }

    // 2. 用户登录鉴权 UserInfoAuth
    AuthUserInfo userInfo;
    errMsg = MesHttpPost::Instance()->UserInfoAuth(user, pwd, workOrderId, processId, userInfo);
    if (!errMsg.isEmpty()){
        errMsg = "用户登录:" + errMsg;
        return false;
    }

    // 3. 设置工序，获取processKey
    QString processKey;
    errMsg = MesHttpPost::Instance()->ConfigProcess(processId, processKey);
    if (!errMsg.isEmpty()){
        errMsg = "用户登录:" + errMsg;
        return false;
    }

    // 4. 获取生产信息
    ProductionInfo prodInfo;
    errMsg = MesHttpPost::Instance()->QueryProductionInfo(prodInfo);
    if (!errMsg.isEmpty()){
        errMsg = "获取生产信息:" + errMsg;
        return false;
    }

    // 5.获取物料信息
    if(m_load){
        errMsg = MesHttpPost::Instance()->QueryProcessMaterial(m_materialInfo);
        if (!errMsg.isEmpty()){
            errMsg = "获取物料信息:" + errMsg;
            return false;
        }
    }

    // 5. 启动周期心跳
//    m_heartTimer.start();

    //6.通知治具检查和自检
    emit SignalCheckFixtureNeedAndSelfCheck();

    g_success = true;
    return true;
}
