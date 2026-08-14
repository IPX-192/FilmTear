#include "ShieldTableModel.h"
#include <QColor>
Q_DECLARE_METATYPE(ShieldData)
// Constructor
ShieldTableModel::ShieldTableModel(QObject *parent)
    : QAbstractTableModel(parent)
{
    qRegisterMetaType<ShieldData>("ShieldData");
    // Initialize m_dataUserRole with all items having checked=false (shielded state)
    // This will be done when adding rows through addRow method
}

// Returns the number of rows
int ShieldTableModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
        
    return m_data.size();
}

// Returns the number of columns (fixed to 3: Name, Shield, Unshield)
int ShieldTableModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
        
    return COL_Shield_MAX; // Name, Shield, Unshield
}

// Gets data
QVariant ShieldTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_data.size() || index.row() < 0)
        return QVariant();

    if (role == Qt::DisplayRole) {
        // For display role, return the appropriate color based on the button state
        const ShieldData &item = m_data[index.row()];
        
        switch (index.column()) {
        case NameCol: // Name column
            return item.name;
        case ShieldCol: // Shield column
            return  QStringLiteral("屏蔽") ;
        case UnshieldCol: // Unshield column
            return  QStringLiteral("使用") ;
        case ControlPointCol: // Control Point column
            // Show the control point value as string
            return item.pointAddress;
        default:
            return QVariant();
        }
    }
    
    if (role == Qt::EditRole) {
        // For edit role, return the boolean state of the button
        const ShieldData &item = m_data[index.row()];
        switch (index.column()) {
        case ShieldCol: // Shield column - now returns inverse of checked
            return !item.checked;  // If checked (meaning "in use"), then shield is off
        case UnshieldCol: // Unshield column - now returns checked value
            return item.checked;   // If checked (meaning "in use"), then unshield is on
        case NameCol: // Name column
            return item.name;
        default:
            return QVariant();
        }
    }
    
    // Adding support for Qt::TextAlignmentRole to achieve text centering
    if (role == Qt::TextAlignmentRole) {
        return Qt::AlignCenter;
    }
    
    // Add support for Qt::BackgroundRole to return the button's background color
//    if (role == Qt::BackgroundRole) {
//        const ShieldData &item = m_data[index.row()];
//        switch (index.column()) {
//        case ShieldCol: // Shield column
//            return !item.checked ? QColor(76, 175, 80) : QColor(128, 128, 128); // Green for active, gray for inactive
//        case UnshieldCol: // Unshield column
//            return item.checked ? QColor(76, 175, 80) : QColor(128, 128, 128); // Green for active, gray for inactive
//        default:
//            return QColor(128, 128, 128); // Default gray
//        }
//    }

    // Add support for Qt::TextColorRole to set text color based on button state
    if (role == Qt::TextColorRole) {
        const ShieldData &item = m_data[index.row()];
        switch (index.column()) {
        case ShieldCol: // Shield column
            return !item.checked ? QColor(Qt::white) : QColor(Qt::black);
        case UnshieldCol: // Unshield column
            return item.checked ? QColor(Qt::white) : QColor(Qt::black);
        default:
            return QColor(Qt::black);
        }
    }

    return QVariant();
}

// Gets header data
QVariant ShieldTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QAbstractTableModel::headerData(section, orientation, role);

    if (orientation == Qt::Horizontal) {
        switch (section) {
        case NameCol:
            return QString(QStringLiteral("名称"));
        case ShieldCol:
            return QString(QStringLiteral("屏蔽"));
        case UnshieldCol:
            return QString(QStringLiteral("使用"));
        case ControlPointCol:  // Added header for Control Point column
            return QString(QStringLiteral(" Control Point "));
        default:
            return QVariant();
        }
    }
    
    return QAbstractTableModel::headerData(section, orientation, role);
}

// Sets data
bool ShieldTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || index.row() >= m_data.size())
        return false;

    ShieldData &item = m_data[index.row()];
    
    if (role == Qt::EditRole) {
        switch (index.column()) {
        case NameCol: // Name
            item.name = value.toString();
            break;
        case ShieldCol: // Shield - when shield is clicked, set checked to false
            item.checked = !value.toBool();  // If shield is activated (true), then checked should be false
            break;
        case UnshieldCol: // Unshield - when unshield is clicked, set checked to true
            item.checked = value.toBool();   // If unshield is activated (true), then checked should be true
            break;
        case ControlPointCol: // Control Point
            item.pointAddress = value.toString();
            break;
        default:
            return false;
        }
    } 
    else if (role == Qt::BackgroundRole) {
        // Handle background color changes if needed
        // Currently just returning true without any action since we handle colors via painting
        return true;
    }
    else {
        return false;
    }
    
    emit dataChanged(index, index, QVector<int>() << Qt::DisplayRole << Qt::EditRole << Qt::BackgroundRole << Qt::TextColorRole);
    return true;
}

// Sets item flags
Qt::ItemFlags ShieldTableModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
}

// Adds row data
void ShieldTableModel::addRow(const QString &name, bool checked, QString controlPoint)
{
    beginInsertRows(QModelIndex(), m_data.size(), m_data.size());

    m_data.append(ShieldData(name, checked, controlPoint));
    // Initialize user role data with checked=false (shielded state) by default
    m_dataUserRole.append(ShieldData(name, false, controlPoint)); 
    endInsertRows();
}

// Update shield status method - fix variable names to match the struct
void ShieldTableModel::updateShieldStatus(int row, bool isShielded)
{
    if (row < 0 || row >= m_data.size())
        return;
        
    m_data[row].checked = !isShielded;  // If shielded is true, then checked (in use) is false
    
    QModelIndex topLeft = index(row, ShieldCol);
    QModelIndex bottomRight = index(row, UnshieldCol);
    emit dataChanged(topLeft, bottomRight, QVector<int>() << Qt::DisplayRole);
}

// Update unshield status method - fix variable names to match the struct
void ShieldTableModel::updateUnshieldStatus(int row, bool isUnshielded)
{
    if (row < 0 || row >= m_data.size())
        return;
        
    m_data[row].checked = isUnshielded;  // If unshielded is true, then checked (in use) is true
    
    QModelIndex topLeft = index(row, ShieldCol);
    QModelIndex bottomRight = index(row, UnshieldCol);
    emit dataChanged(topLeft, bottomRight, QVector<int>() << Qt::DisplayRole);
}


bool ShieldTableModel::removeRow(int row, const QModelIndex &parent)
{
    if (row < 0 || row >= m_data.size())
        return false;
        
    beginRemoveRows(parent, row, row);
    m_data.removeAt(row);
    m_dataUserRole.removeAt(row); // Also remove from user role data
    endRemoveRows();
    
    return true;
}

ShieldData* ShieldTableModel::rowData(int row)
{
    if(row>=m_data.size()) return nullptr;
    return &m_data[row];
}

// Add new methods for saving and refreshing data
void ShieldTableModel::saveData()
{
    m_dataUserRole = m_data; // Copy current data to user role data
}

void ShieldTableModel::refreshData()
{
    beginResetModel();
    m_data = m_dataUserRole; // Restore data from user role data
    endResetModel();
}
