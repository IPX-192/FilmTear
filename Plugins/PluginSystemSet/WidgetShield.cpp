#include "WidgetShield.h"
#include <QVBoxLayout>
#include <QHeaderView>
#include <QPushButton>
#include <QApplication>
#include <QDesktopWidget>
#include <QFormLayout>
#include <QLabel>
#include <QMessageBox>
#include <QDir>
#include <QStandardPaths>
#include <QDebug>
#include <QSettings>
#include "VisAppBus.h"
#include "Shield/ButtonDelegate.h"
#include "Shield/ShieldTableModel.h"


// Callback function implementation
static bool buttonClickHandler(ShieldData *data, int row, int col) {

    return true; // Return success
}

WidgetShield::WidgetShield(QWidget *parent, const QString& iniFilePath)
    : QWidget(parent), m_model(nullptr)
{
    // Store the INI file path - if not provided, use default
    if (!iniFilePath.isEmpty()) {
        m_iniFilePath = iniFilePath;
    } else {
        // Use executable directory /config/btnCtrl.ini as default
        QString exeDir = QCoreApplication::applicationDirPath();
        m_iniFilePath = QDir(exeDir).absoluteFilePath("Config/Shield.ini");
    }
    
    InitializeUI();
    InitializeData(); // Initialize data from image
}

WidgetShield::~WidgetShield()
{
    if (m_model) {
        delete m_model;
    }
}

void WidgetShield::InitializeUI()
{
    // Create main layout
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    // Create the table view
    m_tableView = new QTableView(this);
    QFont font = m_tableView->font();
    font.setPointSize(14);
    m_tableView->setFont(font);
    // Create and set the model
    m_model = new ShieldTableModel(this);
    m_tableView->setModel(m_model);

    // Set up delegates for the "Shield" and "Unshield" columns with callback function
    ButtonDelegate *shieldDelegate = new ButtonDelegate(buttonClickHandler, this);
    ButtonDelegate *unshieldDelegate = new ButtonDelegate(buttonClickHandler, this);
    
    m_tableView->setItemDelegateForColumn(ShieldTableModel::ShieldCol, shieldDelegate); // Shield column
    m_tableView->setItemDelegateForColumn(ShieldTableModel::UnshieldCol, unshieldDelegate);   // Unshield column

    // Enable selection for the table view
    m_tableView->setSelectionBehavior(QAbstractItemView::SelectRows); // Select entire rows
    m_tableView->setSelectionMode(QAbstractItemView::SingleSelection); // Allow single row selection

    // Hide row headers and grid lines
    m_tableView->setShowGrid(false);
    m_tableView->horizontalHeader()->setVisible(true);
    m_tableView->verticalHeader()->setVisible(false);
    m_tableView->setColumnHidden(ShieldTableModel::ControlPointCol,true);

    // Set fixed row height
    m_tableView->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    m_tableView->verticalHeader()->setDefaultSectionSize(60); // Set row height to 40 pixels

    // Configure headers
    m_tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    mainLayout->addWidget(m_tableView);  // Table view remains below

    setLayout(mainLayout);
}


void WidgetShield::InitializeData()
{
    ShieldParam& paramShield=GlobalParam->systemParam.shieldParam;
    QVector<ShieldInfoPtr> initialData = {
       {u8"安全门", &paramShield.safeDoor, ""},
       //{u8"气压报警", &paramShield.airWaring, ""},
       {u8"治具1", &paramShield.turntable[0], ""},
       {u8"治具2", &paramShield.turntable[1], ""},
       {u8"治具3", &paramShield.turntable[2], ""},
       {u8"治具4", &paramShield.turntable[3], ""},
       {u8"MES", &paramShield.mes, ""},
    };

    for (auto it = initialData.begin(); it != initialData.end(); ++it) {
        const auto& item = *it;
        m_shieldDataMap[item.name] = QPair<bool*, QString>(item.checked, item.pointAddress);
        InsertRow(item.name, *item.checked, item.pointAddress);
    }
}

void WidgetShield::LoadUIParam()
{
    if (!m_model) return;
    
    QSettings settings(m_iniFilePath, QSettings::IniFormat);
    settings.setIniCodec("UTF-8");
    ShieldTableModel* model = qobject_cast<ShieldTableModel*>(m_model);
    if (!model) return;
    // Read data from INI file
    int size = settings.beginReadArray("shield_data");
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        QString value = settings.value(QString("shield%1").arg(i + 1)).toString();
        QStringList listValue = value.split(",");
        if(listValue.size()<2)continue;
        QString name = listValue.at(0);
        bool status = listValue.at(1).toInt();
        if(!m_shieldDataMap.contains(name))continue;
        *m_shieldDataMap[name].first = status;
        // Set the appropriate state based on saved status value
        QModelIndex modelIndex = model->index(i, status ? ShieldTableModel::ShieldCol : ShieldTableModel::UnshieldCol);
		model->setData(modelIndex, true, Qt::EditRole);
    }
    settings.endArray();
}

void WidgetShield::SaveUIParam()
{
    if (!m_model) return;
    
    QSettings settings(m_iniFilePath, QSettings::IniFormat);
    settings.setIniCodec("UTF-8");
    ShieldTableModel* model = qobject_cast<ShieldTableModel*>(m_model);
    if (!model) return;
    
    // Save data to INI file
    settings.beginWriteArray("shield_data", model->rowCount());
    for (int i = 0; i < model->rowCount(); ++i) {
        settings.setArrayIndex(i);
        QString name = model->data(model->index(i, ShieldTableModel::NameCol)).toString();
        bool checked = model->data(model->index(i, ShieldTableModel::ShieldCol), Qt::EditRole).toBool();
        settings.setValue(QString("shield%1").arg(i + 1), name + "," + QString::number(checked));
		*m_shieldDataMap[name].first = checked;
    }
    settings.endArray();
}

void  WidgetShield::UpdateParamToUI()
{
    LoadUIParam();
}

void WidgetShield::InsertRow(const QString &name, bool checked, const QString &address)
{
    ShieldTableModel *model = qobject_cast<ShieldTableModel*>(m_model);
    if (model) {
        // Parse address string to integer if not empty
        model->addRow(name, checked, address);  // Using !isShielded as the second parameter since original expected both
        // Connect signals to handle button clicks
        connect(model, &ShieldTableModel::dataChanged, this, [this](const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles) {
            if (roles.contains(Qt::DisplayRole)) {
                m_tableView->viewport()->update();
            }
        });
    }
}
