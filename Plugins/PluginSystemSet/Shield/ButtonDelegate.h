#ifndef BUTTONDELEGATE_H
#define BUTTONDELEGATE_H

#include <QStyledItemDelegate>
#include <QWidget>
#include <QModelIndex>
#include "ShieldTableModel.h"

// Forward declaration
class QPainter;
class QStyleOptionViewItem;

// Define callback type for external handling
typedef bool (*ButtonClickCallback)(ShieldData*, int, int);

class ButtonDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit ButtonDelegate(ButtonClickCallback callback = nullptr, QObject *parent = nullptr);
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override;

private:
    ButtonClickCallback m_callback;
};

#endif // BUTTONDELEGATE_H
