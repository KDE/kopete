/*
    pipesdelegate.cpp

    Copyright (c) 2007      by Charles Connell <charles@connells.org>

    Kopete    (c) 2007      by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/
#include "pipesdelegate.h"

#include "pipesmodel.h"
#include "pipesplugin.h"

#include <QModelIndex>
#include <KComboBox>
#include <QCheckBox>
#include <klocale.h>

PipesDelegate::PipesDelegate ( QObject *parent )
		: QItemDelegate ( parent )
{
}

QWidget *PipesDelegate::createEditor ( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
	// make a combobox so they can change the Direction column
	if ( index.column() == DirectionColumn ) // direction
	{
		KComboBox * editor = new KComboBox ( false/*readwrite*/, parent );
		editor->insertItem ( 0, i18nc ( "adjective decribing instant message", "Inbound" ) );
		editor->insertItem ( 1, i18nc ( "adjective decribing instant message", "Outbound" ) );
		editor->insertItem ( 2, i18nc ( "adjective decribing instant message directions inbound and outbound", "Both Directions" ) );
		return editor;
	}
	// make a combobox so they can change the Contents column
	else if ( index.column() == ContentsColumn ) // pipe contents
	{
		KComboBox * editor = new KComboBox ( false/*readwrite*/, parent );
		editor->insertItem ( 0, i18n ( "HTML Message Body" ) );
		editor->insertItem ( 1, i18n ( "Plain Text Message Body" ) );
		editor->insertItem ( 2, i18n ( "Kopete Message XML" ) );
		return editor;
	}
	// make a checkbox so they can change the Enablec column
	else if ( index.column() == EnabledColumn ) // enabled/disabled checkbox
	{
		QCheckBox * editor = new QCheckBox ( parent );
		return editor;
	}
	else
		return QItemDelegate::createEditor ( parent, option, index );
}

void PipesDelegate::setEditorData ( QWidget *editor, const QModelIndex &index ) const
{
	if ( index.column() == ContentsColumn )
	{
		const int value = index.model()->data ( index, Qt::DisplayRole ).toInt();
		KComboBox *comboBox = static_cast<KComboBox*> ( editor );
		switch ( value )
		{
			// translate enums into combobox indexes
			case PipesPlugin::HtmlBody: comboBox->setCurrentIndex ( 0 ); break;
			case PipesPlugin::PlainBody: comboBox->setCurrentIndex ( 1 ); break;
			case PipesPlugin::Xml: comboBox->setCurrentIndex ( 2 ); break;
		}
	}
	else if ( index.column() == DirectionColumn )
	{
		int value = index.model()->data ( index, Qt::DisplayRole ).toInt();
		KComboBox *comboBox = static_cast<KComboBox*> ( editor );
		switch ( value )
		{
			// translate enums into combobox indexes
			case PipesPlugin::Inbound: comboBox->setCurrentIndex ( 0 ); break;
			case PipesPlugin::Outbound: comboBox->setCurrentIndex ( 1 ); break;
			case PipesPlugin::BothDirections: comboBox->setCurrentIndex ( 2 ); break;
		}
	}
	else if ( index.column() == EnabledColumn )
	{
		const int value = index.model()->data ( index, Qt::CheckStateRole ).toInt();
		QCheckBox *checkBox = static_cast<QCheckBox*> ( editor );
		checkBox->setCheckState ( ( Qt::CheckState ) value );
	}
	else
		QItemDelegate::setEditorData ( editor, index );
}

void PipesDelegate::setModelData ( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const
{
	if (  index.column() == ContentsColumn )
	{
		KComboBox *comboBox = static_cast<KComboBox*> ( editor );
		const int value = comboBox->currentIndex();
		switch ( value )
		{
			// translate combobox indexes into enums
			case 0: model->setData ( index, PipesPlugin::HtmlBody ); break;
			case 1: model->setData ( index, PipesPlugin::PlainBody ); break;
			case 2: model->setData ( index, PipesPlugin::Xml ); break;
		}
	}
	else if ( index.column() == DirectionColumn )
	{
		KComboBox *comboBox = static_cast<KComboBox*> ( editor );
		const int value = comboBox->currentIndex();
		switch ( value )
		{
			// translate combobox indexes into enums
			case 0: model->setData ( index, PipesPlugin::Inbound ); break;
			case 1: model->setData ( index, PipesPlugin::Outbound ); break;
			case 2: model->setData ( index, PipesPlugin::BothDirections ); break;
		}
	}
	else if ( index.column() == EnabledColumn )
	{
		QCheckBox *checkBox = static_cast<QCheckBox*> ( editor );
		const int value = checkBox->isChecked();
		model->setData ( index, value, Qt::CheckStateRole);
	}
	else
		QItemDelegate::setModelData ( editor, model, index );
}

void PipesDelegate::updateEditorGeometry ( QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex & /* index */ ) const
{
	editor->setGeometry ( option.rect );
}

void PipesDelegate::paint ( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
	QString text;
	QStyleOptionViewItem myOption = option;

	myOption.displayAlignment = Qt::AlignLeft | Qt::AlignVCenter;
	myOption.textElideMode = Qt::ElideRight;
	
	if ( index.column() == DirectionColumn )
	{
		PipesPlugin::PipeDirection direction = (PipesPlugin::PipeDirection)index.model()->data ( index, Qt::DisplayRole ).toInt();
		// translate enums into displayable strings and paint them
		if ( direction == PipesPlugin::Inbound )
			text = i18nc ( "adjective decribing an instant message", "Inbound" );
		else if ( direction == PipesPlugin::Outbound )
			text = i18nc ( "adjective decribing an instant message", "Outbound" );
		else if ( direction == PipesPlugin::BothDirections )
			text = i18nc ( "adjective decribing instant message directions inbound and outbound", "Both Directions" );
		drawDisplay ( painter, myOption, myOption.rect, text );
	}
	else if ( index.column() == ContentsColumn )
	{
		PipesPlugin::PipeContents contents = (PipesPlugin::PipeContents)index.model()->data ( index, Qt::DisplayRole ).toInt();
		// translate enums into displayable strings and paint them
		if ( contents == PipesPlugin::HtmlBody )
			text = i18n ( "HTML Message Body" );
		else if ( contents == PipesPlugin::PlainBody )
			text = i18n ( "Plain Text Message Body" );
		else if ( contents == PipesPlugin::Xml )
			text = i18n ( "Kopete Message XML" );
		drawDisplay ( painter, myOption, myOption.rect, text );
	}
	else
		QItemDelegate::paint ( painter, option, index );
}

