/*
    kopeteidentitystatusbaricon.cpp  -  Kopete Identity StatusBar Dock Icon

    Copyright (c) 2007      by Gustavo Pichorim Boiko <gustavo.boiko@kdemail.net>
    Copyright (c) 2001-2003 by Duncan Mac-Vicar Prett <duncan@kde.org>

    Kopete    (c) 2002-2007      by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "kopeteidentitystatusbaricon.h"

#include <KDebug>
#include <QMouseEvent>
#include <QLabel>
#include <QEvent>
#include <QCursor>

KopeteIdentityStatusBarIcon::KopeteIdentityStatusBarIcon( Kopete::Identity *identity, QWidget *parent )
: QLabel( parent )
{
	setFixedSize ( 16, 16 );
	setCursor(QCursor(Qt::PointingHandCursor));
	show();

	m_identity = identity;
}

KopeteIdentityStatusBarIcon::~KopeteIdentityStatusBarIcon()
{
}

void KopeteIdentityStatusBarIcon::mousePressEvent( QMouseEvent *me )
{
	if( me->button() == Qt::RightButton )
	{
		emit rightClicked( m_identity, QPoint( me->globalX(), me->globalY() ) );
	}
	else if( me->button() == Qt::LeftButton )
	{
		emit leftClicked( m_identity, QPoint( me->globalX(), me->globalY() ) );
	}
}

#include "kopeteidentitystatusbaricon.moc"

// vim: set noet ts=4 sts=4 sw=4:

