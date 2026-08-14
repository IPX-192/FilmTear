#include "ButtonDelegate.h"
#include <QPainter>
#include <QStyleOptionViewItem>
#include <QStyleOptionButton>
#include <QApplication>
#include <QStyle>
#include "ShieldTableModel.h"
#include <QDebug> 
#include <QMouseEvent>
ButtonDelegate::ButtonDelegate(ButtonClickCallback callback, QObject *parent)
    : QStyledItemDelegate(parent), m_callback(callback)
{
}

void ButtonDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
	const QModelIndex &index) const
{
	// Don't draw the default background - this removes the grey background
	// Just prepare for button drawing below

	// Create a button option for drawing the actual button
	QStyleOptionButton buttonOption;
	buttonOption.rect = option.rect.adjusted(4, 4, -4, -4); // Add some padding

															// Set button text based on the model data
	QString buttonText = index.data().toString();
	if (!buttonText.isEmpty()) {
		buttonOption.text = buttonText;
	}
	else {
		// Default text based on column
		if (index.column() == ShieldTableModel::ShieldCol) { // Shield column
            buttonOption.text = QStringLiteral("Shield");
		}
		else if (index.column() == ShieldTableModel::UnshieldCol) { // Use column
            buttonOption.text = QStringLiteral("Used");
		}
	}

	buttonOption.state |= QStyle::State_Enabled;

	// Get the actual data from the model for the entire row
    bool isChecked = index.sibling(index.row(), ShieldTableModel::ShieldCol).data(Qt::EditRole).toBool();
	
	// Determine the button state and apply appropriate styling
	bool isActive = false;
	if (index.column() == ShieldTableModel::ShieldCol) {
        isActive = isChecked; // Shield button is active when not checked
	}
	else if (index.column() == ShieldTableModel::UnshieldCol) {
        isActive = !isChecked; // Unshield button is active when checked
	}

	// Apply styling based on the button state using stylesheet-like approach
	if (isActive) {
		// Active button style - green background with white text
		buttonOption.features = QStyleOptionButton::DefaultButton; // This helps with styling
																   // Instead of setting palette brushes directly, we'll use the style to draw appropriately
		painter->save();
		painter->setPen(QPen(QColor(76, 175, 80), 2)); // Border color
		painter->setBrush(QBrush(QColor(76, 175, 80))); // Background color
		painter->drawRoundedRect(buttonOption.rect, 3, 3);

		// Draw text in white for active button
		painter->setPen(QColor(Qt::white));
		QRect textRect = buttonOption.rect.adjusted(10, 5, -10, -5);
		// Set font size before drawing text
		QFont font = painter->font();
		font.setPointSize(20); // You can adjust this value as needed
		painter->setFont(font);
		painter->drawText(textRect, Qt::AlignCenter, buttonOption.text);
		painter->restore();
	}
	else {
		// Inactive button style - gray background with black text
		painter->save();
		painter->setPen(QPen(QColor(128, 128, 128), 2)); // Border color
		painter->setBrush(QBrush(QColor(192, 192, 192))); // Background color
		painter->drawRoundedRect(buttonOption.rect, 3, 3);

		// Draw text in black for inactive button
		painter->setPen(QColor(Qt::black));
		QRect textRect = buttonOption.rect.adjusted(10, 5, -10, -5);
		// Set font size before drawing text
		QFont font = painter->font();
		font.setPointSize(20); // You can adjust this value as needed
		painter->setFont(font);
		painter->drawText(textRect, Qt::AlignCenter, buttonOption.text);
		painter->restore();
	}
}


QSize ButtonDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    return QSize(50, 30); // Fixed size for buttons
}

// Handle ButtonDelegate mouse click events
bool ButtonDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if (event->type() == QEvent::MouseButtonRelease) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->button() == Qt::LeftButton && option.rect.contains(mouseEvent->pos())) {
            // Get the current row and column
            int row = index.row();
            int col = index.column();

            // Get the current model
            ShieldTableModel *shieldModel = qobject_cast<ShieldTableModel*>(model);
            if (!shieldModel) {
                return false;
            }
            
            // Get the current row's ShieldData
            if (row < 0 || row >= shieldModel->rowCount()) {
                return false;
            }
            // Toggle the checked state
            if (col == ShieldTableModel::ShieldCol) {
                // Toggle checked state for shield button
                model->setData(index, true, Qt::EditRole); 

                // Force refresh of the view
                emit const_cast<QAbstractItemModel*>(model)->dataChanged(index, index, QVector<int>() << Qt::DisplayRole << Qt::BackgroundRole << Qt::EditRole);
                
				auto bIndex = model->index(index.row(),ShieldTableModel::UnshieldCol);
                model->setData(bIndex, false, Qt::EditRole);
				emit const_cast<QAbstractItemModel*>(model)->dataChanged(bIndex, bIndex, QVector<int>() << Qt::DisplayRole << Qt::BackgroundRole << Qt::EditRole); 
               
            } 
			else if (col == ShieldTableModel::UnshieldCol) {
                                // Toggle checked state for shield button
                model->setData(index, true, Qt::EditRole); 

                // Force refresh of the view
                emit const_cast<QAbstractItemModel*>(model)->dataChanged(index, index, QVector<int>() << Qt::DisplayRole << Qt::BackgroundRole << Qt::EditRole);
                
				auto bIndex = model->index(index.row(),ShieldTableModel::ShieldCol);
                model->setData(bIndex, false, Qt::EditRole);
				emit const_cast<QAbstractItemModel*>(model)->dataChanged(bIndex, bIndex, QVector<int>() << Qt::DisplayRole << Qt::BackgroundRole << Qt::EditRole); 
            }
			 // Call the callback function if available
			if (m_callback) {
				ShieldData *data = shieldModel->rowData(row);
				if (data) {
					m_callback(data, row, col);
				}
			}
        }
    }
    return true; // Return true to indicate event was handled
}
