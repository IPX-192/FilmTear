#include "ItemDelegate.h"
#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QCheckBox>
#include <QApplication>
#include <qDebug>
#include "ParamManager.h"

ItemDelegate::ItemDelegate(QObject *parent) : QStyledItemDelegate(parent)
{

}


QWidget *ItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    int col=index.column();
    if (col == 0) return nullptr;
	QWidget *pWidget = nullptr;
	DelegateType delegateType = (DelegateType)index.data(Qt::UserRole + 1).toInt();
	QStringList itemList = index.data(Qt::UserRole + 2).toStringList();
	if (delegateType == LineEditDelegate) {
		QLineEdit *editor = new QLineEdit(parent);
		editor->setFixedHeight(option.rect.height());
		return editor;
	}
	else if (delegateType == SpinBoxDelegate) {
		QSpinBox *spinBox = new QSpinBox(parent);
		spinBox->setFixedHeight(option.rect.height());
		if (itemList.size() > 1)
			spinBox->setRange(itemList[0].toInt(), itemList[1].toInt());

		return spinBox;
	}
	else if (delegateType == DoubleSpinDelegate) {
		QDoubleSpinBox *spinBox = new QDoubleSpinBox(parent);
		spinBox->setFrame(false);
	//	spinBox->setFixedHeight(option.rect.height());
		spinBox->setDecimals(3);
		spinBox->setMinimum(0);
		spinBox->setMaximum(10000);
		if (itemList.size() > 1)
			spinBox->setRange(itemList[0].toFloat(), itemList[1].toFloat());
		return spinBox;
	}
	else if (delegateType == ComboBoxDelegate) {
		QComboBox *comboBox = new QComboBox(parent);
        comboBox->setStyleSheet("QComboBox{font:20px;height: 50px;}");
        comboBox->addItems(GlobalParam->recipeProduct.vecRecipeNode[col].second->listRecipe);

		return comboBox;
	}

	return pWidget;
}

void ItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
	DelegateType delegateType = (DelegateType)index.data(Qt::UserRole + 1).toInt();
	if (delegateType == LineEditDelegate) {
		QString value = index.model()->data(index, Qt::EditRole).toString();
		QLineEdit *lineEdit = static_cast<QLineEdit*>(editor);
		lineEdit->setText(value);
		return;
	}
	else if (delegateType == SpinBoxDelegate) {
		int value = index.model()->data(index, Qt::EditRole).toInt();
		QSpinBox* spinBox = static_cast<QSpinBox*>(editor);
		spinBox->setValue(value);
		return;
	}
	else if (delegateType == DoubleSpinDelegate) {
		double value = index.model()->data(index, Qt::EditRole).toDouble();
		QDoubleSpinBox *spinBox = static_cast<QDoubleSpinBox*>(editor);
		spinBox->setValue(value);
		return;
	}
	else if (delegateType == ComboBoxDelegate) {
		QString value = index.model()->data(index, Qt::EditRole).toString();
		QComboBox *comboBox = static_cast<QComboBox*>(editor);
		comboBox->setCurrentText(value);
		return;
	}
}

void ItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
	QString strValue;
	DelegateType delegateType = (DelegateType)index.data(Qt::UserRole + 1).toInt();
	if (delegateType == LineEditDelegate) {
		QLineEdit *lineEdit = static_cast<QLineEdit*>(editor);
		strValue = lineEdit->text();
		model->setData(index, strValue, Qt::EditRole);
	}
	else if (delegateType == SpinBoxDelegate) {
		QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
		model->setData(index, spinBox->value(), Qt::EditRole);
	}
	else if (delegateType == DoubleSpinDelegate) {
		QDoubleSpinBox *spinBox = static_cast<QDoubleSpinBox*>(editor);
		spinBox->interpretText();
		model->setData(index, spinBox->text(), Qt::EditRole);
	}
	else if (delegateType == ComboBoxDelegate) {
		QComboBox *comboBox = static_cast<QComboBox*>(editor);
		strValue = comboBox->currentText();
		model->setData(index, strValue, Qt::EditRole);
	}
}

void ItemDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(index)
	editor->setGeometry(option.rect);
}
