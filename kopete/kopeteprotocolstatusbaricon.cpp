/*
    KopeteProtocolStatusBarIcon.cpp  -  Kopete StatusBar Dock Icon

    Copyright (c) 2001-2003 by Duncan Mac-Vicar Prett   <duncan@kde.org>

    Kopete    (c) 2002 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "kopeteprotocolstatusbaricon.h"
#include "kopeteprotocol.h"
#include <qpixmap.h>
#include <kdebug.h>

//#include <kdebug.h>

KopeteProtocolStatusBarIcon::KopeteProtocolStatusBarIcon( KopeteProtocol *proto, QWidget *parent,
	const char *name )
: QLabel( parent, name )
{
// 	kdDebug(14000) << "[KopeteProtocolStatusBarIcon] Setting Initial Protocol Icon" << endl;
	//setMask(initialPixmap->mask());
	//setPixmap( KopeteOnlineStatus( KopeteOnlineStatus::Unknown, 0, proto, 0, "status_unknown", QString::null, QString::null ).protocolIcon() );
	setPixmap( proto->status().protocolIcon() );

	setFixedSize ( 16, 16 );
	show();

	m_proto = proto;
}

KopeteProtocolStatusBarIcon::~KopeteProtocolStatusBarIcon()
{
}

void KopeteProtocolStatusBarIcon::mousePressEvent( QMouseEvent *me )
{
	if( me->button() == QEvent::RightButton )
	{
		emit rightClicked( m_proto, QPoint( me->globalX(), me->globalY() ) );
	}
}

#include "kopeteprotocolstatusbaricon.moc"

// vim: set noet ts=4 sts=4 sw=4:

