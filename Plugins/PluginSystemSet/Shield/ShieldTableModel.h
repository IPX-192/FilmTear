#ifndef SHIELDTABLEMODEL_H
#define SHIELDTABLEMODEL_H

#include <QAbstractTableModel>
#include <QList>
#include <QStringList>

//union ControlPoint {
//    int address;
//    bool status;
//    
//    ControlPoint(bool s = false) : status(s) {}  // Default to false as requested
//};

struct ShieldData {
    QString name;
    bool checked;  // Single variable replacing unUsedChecked and usedChecked
    QString pointAddress;  // New control point member
    
    ShieldData(const QString &n = "", bool chk = false, QString cp = "")
        : name(n), checked(chk), pointAddress(cp) {}
};

// Shield table model class - Implements a data model with three columns: name, shield, and unshield
class ShieldTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    // Constructor
    explicit ShieldTableModel(QObject *parent = nullptr);

    // Define enum for column indices
    enum ColumnIndex {
        NameCol = 0,
        ShieldCol = 1,
        UnshieldCol = 2,
        ControlPointCol = 3,  // New control point column
        COL_Shield_MAX
    };

    // Implementation of virtual functions inherited from QAbstractTableModel
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool removeRow(int row, const QModelIndex &parent = QModelIndex());

    // Method to add row data - updated to include control point parameter
    void addRow(const QString &name, bool checked = false, QString controlPoint=""); // Default usage
    // Update shield status
    void updateShieldStatus(int row, bool isShielded);
    // Update unshield status
    void updateUnshieldStatus(int row, bool isUnshielded);


    // Row data
    ShieldData* rowData(int row);
    QVector<ShieldData> getData() { return m_data; }
    
    // Add method declarations for saveData and refreshData
    void saveData();
    void refreshData();

private:
    QVector<ShieldData> m_data;
    QVector<ShieldData> m_dataUserRole;

signals:
    void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles = QVector<int>());
};

#endif // SHIELDTABLEMODEL_H