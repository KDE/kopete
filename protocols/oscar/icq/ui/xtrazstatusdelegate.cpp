/*
    xtrazstatusdelegate.cpp  -  Xtraz Status Delegate

    Copyright (c) 2007 by Roman Jarosz <kedgedev@centrum.cz>
    Kopete    (c) 2007 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "xtrazstatusdelegate.h"

#include <QLineEdit>

#include <kicon.h>

#include "editorwithicon.h"

namespace Xtraz
{

StatusDelegate::StatusDelegate( const QList<QIcon> &icons, QObject *parent )
	: QItemDelegate( parent ), mIcons( icons )
{
}

QWidget *StatusDelegate::createEditor( QWidget *parent, const QStyleOptionViewItem&, const QModelIndex &index ) const
{
	switch ( index.column() )
	{
	case 0:
		return new EditorWithIcon( mIcons, parent );
	case 1:
		{
			QLineEdit* lineEdit = new QLineEdit( parent );
			lineEdit->setFrame( false );
			return lineEdit;
		}
	default:
		return 0;
	}
}

void StatusDelegate::setEditorData( QWidget *editor, const QModelIndex &index ) const
{
	if ( EditorWithIcon *e = qobject_cast<EditorWithIcon*>(editor) )
	{
		e->setText( index.model()->data( index, Qt::DisplayRole ).toString() );
		e->setIconIndex( index.model()->data( index, Qt::UserRole ).toInt() );
	}
	else if ( QLineEdit *e = qobject_cast<QLineEdit*>(editor) )
	{
		e->setText( index.model()->data( index, Qt::DisplayRole ).toString() );
	}
}

void StatusDelegate::setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const
{
	if ( EditorWithIcon *e = qobject_cast<EditorWithIcon*>(editor) )
	{
		model->setData( index, e->text(), Qt::EditRole );
		model->setData( index, e->iconIndex(), Qt::UserRole );
	}
	else if ( QLineEdit *e = qobject_cast<QLineEdit*>(editor) )
	{
		model->setData( index, e->text(), Qt::EditRole );
	}
}

void StatusDelegate::updateEditorGeometry( QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex& ) const
{
    editor->setGeometry( option.rect );
}

}

#include "xtrazstatusdelegate.moc"
