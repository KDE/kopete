/*
    kopeteaccountstatusbaricon.cpp  -  Kopete Account StatusBar Dock Icon

    Copyright (c) 2001-2003 by Duncan Mac-Vicar Prett   <duncan@kde.org>

    Kopete    (c) 2002-2003      by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "kopeteaccountstatusbaricon.h"
#include "qcursor.h"

#include <kdebug.h>

//#include <kdebug.h>

KopeteAccountStatusBarIcon::KopeteAccountStatusBarIcon( Kopete::Account *acc, QWidget *parent,
	const char *name )
: QLabel( parent, name )
{
// 	kdDebug(14000) << "[KopeteAccountStatusBarIcon] Setting Initial Protocol Icon" << endl;
	//setMask(initialPixmap->mask());
	//setPixmap( Kopete::OnlineStatus( Kopete::OnlineStatus::Unknown, 0, proto, 0, "status_unknown", QString::null, QString::null ).protocolIcon() );
	//setPixmap( proto->status().protocolIcon() );

	setFixedSize ( 16, 16 );
	setCursor(QCursor(Qt::PointingHandCursor));
	show();

	m_account = acc;
}

KopeteAccountStatusBarIcon::~KopeteAccountStatusBarIcon()
{
}

void KopeteAccountStatusBarIcon::mousePressEvent( QMouseEvent *me )
{
	if( me->button() == QEvent::RightButton )
	{
		emit rightClicked( m_account, QPoint( me->globalX(), me->globalY() ) );
	}
	else if( me->button() == QEvent::LeftButton )
	{
		emit leftClicked( m_account, QPoint( me->globalX(), me->globalY() ) );
	}
}

#include "kopeteaccountstatusbaricon.moc"

// vim: set noet ts=4 sts=4 sw=4:

