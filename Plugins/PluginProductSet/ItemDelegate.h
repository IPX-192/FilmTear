/*******************************

Copyright (C), 2020-2, Visensing company.

Filename:
Author:		    王伟
version:		1.0
Date:			2020-8-11

Description:

*******************************/

#ifndef ITEMDELEGATE_H
#define ITEMDELEGATE_H

#include <QStyledItemDelegate>

enum DelegateType{
    LineEditDelegate,
    SpinBoxDelegate,
    DoubleSpinDelegate,
	ComboBoxDelegate
};

enum DelegateValue {
	ItemType= Qt::UserRole + 1,       //代理类型
	ItemRange                         //代理Item项值范围
};

class ItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit ItemDelegate(QObject *parent = nullptr);

public:
	/*void     paint(QPainter *painter, const QStyleOptionViewItem &option,
		const QModelIndex &index) const Q_DECL_OVERRIDE;*/
     QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem &option,
						const QModelIndex &index) const Q_DECL_OVERRIDE;
	 void     setEditorData(QWidget *editor, const QModelIndex &index) const Q_DECL_OVERRIDE;
	 void     setModelData(QWidget *editor, QAbstractItemModel *model,
						const QModelIndex &index) const Q_DECL_OVERRIDE;
	 void     updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
                        const QModelIndex &index) const Q_DECL_OVERRIDE;


private:
    DelegateType   m_nDelegateType;

protected:
    /* bool    editorEvent(QEvent *event, QAbstractItemModel *model,
                       const QStyleOptionViewItem &option, const QModelIndex &index) Q_DECL_OVERRIDE;*/

signals:

public slots:
};

#endif // ITEMDELEGATE_H
