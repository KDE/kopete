/*
    editorwithicon.cpp  -  Editor With Icon

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

#include "editorwithicon.h"

#include <kglobalsettings.h>
#include <QApplication>
#include <QDesktopWidget>

#include  <QVBoxLayout>
#include  <QToolButton>
#include  <QLineEdit>

#include "iconcells.h"

EditorWithIcon::EditorWithIcon( const QList<QIcon> &icons, QWidget *parent )
: QWidget(parent), mIcons(icons)
{
	setAutoFillBackground( true );

	QHBoxLayout *layout = new QHBoxLayout( this );
	layout->setMargin( 0 );
	layout->setSpacing( 0 );

	mIconButton = new QToolButton( this );
	mIconButton->setToolButtonStyle( Qt::ToolButtonIconOnly );
	layout->addWidget( mIconButton );

	mLineEdit = new QLineEdit( this );
	layout->addWidget( mLineEdit );

	connect(mIconButton, &QToolButton::clicked, this, &EditorWithIcon::popupIcons);
	setIconIndex( 0 );
	setTabOrder( mIconButton, mLineEdit );
	setFocusProxy( mLineEdit );
}

void EditorWithIcon::setText( const QString &text )
{
	mLineEdit->setText( text );
	mLineEdit->selectAll();
}

QString EditorWithIcon::text() const
{
	return mLineEdit->text();
}

void EditorWithIcon::setIconIndex( int index )
{
	if ( index < mIcons.size() && index >= 0 )
	{
		mIconIndex = index;
		mIconButton->setIcon( mIcons.at( index ) );
	}
}

void EditorWithIcon::popupIcons()
{
	QFrame *popupFrame = new QFrame( 0, Qt::Popup );
	popupFrame->setAttribute( Qt::WA_DeleteOnClose );
	popupFrame->setFrameStyle(QFrame::StyledPanel);
	popupFrame->setMidLineWidth(2);

	QVBoxLayout *layout =  new QVBoxLayout( popupFrame );
	layout->setSpacing(0);
	layout->setMargin(0);

	IconCells *iconCells = new IconCells( popupFrame );
	iconCells->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
	iconCells->setColumnCount( 7 );
	iconCells->setIcons( mIcons );
	iconCells->setSelectedIndex( mIconIndex );
	connect(iconCells, &IconCells::selected, this, &EditorWithIcon::setIconIndex);
	connect(iconCells, &IconCells::selected, popupFrame, &QFrame::close);
	layout->addWidget( iconCells );

	popupFrame->resize( QSize(150, 100).expandedTo(popupFrame->minimumSizeHint()) );	
	popupFrame->ensurePolished();

	QRect screen = QApplication::desktop()->screenGeometry( mIconButton->rect().bottomLeft() );
	
	QPoint below = mIconButton->mapToGlobal( mIconButton->rect().bottomLeft() );
	int belowHeight = screen.bottom() - below.y();
	QPoint above = mIconButton->mapToGlobal( mIconButton->rect().topLeft() );
	int aboveHeight = above.y() - screen.y();

	QPoint point = below;
	QSize size  = popupFrame->size();

	if ( point.x() + size.width() > screen.right() )
		point.setX( screen.right() - size.width() );

	if ( belowHeight < size.height() )
	{
		point.setY( (	aboveHeight<size.height())?
				screen.bottom():above.y() - size.height() );
	}

	popupFrame->move( point );
	popupFrame->raise();
	popupFrame->show();
	iconCells->setFocus();
}

