/*
    kopeteprotocol.cpp - Kopete Protocol

    Copyright (c) 2002 by Duncan Mac-Vicar Prett <duncan@kde.org>
    Copyright (c) 2002 by Martijn Klingens       <klingens@kde.org>

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

#include "kopeteprotocol.h"

#include "kopetemessagemanagerfactory.h"

#include <kdebug.h>

KopeteProtocol::KopeteProtocol(QObject *parent, const char *name)
    : KopetePlugin( parent, name )
{
}

KopeteProtocol::~KopeteProtocol()
{
}

bool KopeteProtocol::unload()
{
	KopeteMessageManagerFactory::factory()->cleanSessions(this);
	return KopetePlugin::unload();
}


QString KopeteProtocol::statusIcon() const
{
	return m_statusIcon;
}

void KopeteProtocol::setStatusIcon( const QString &icon )
{
	if( icon != m_statusIcon )
	{
		m_statusIcon = icon;
		emit( statusIconChanged( this, icon ) );
	}
}

KActionMenu* KopeteProtocol::protocolActions()
{
	return 0L;
}

const QDict<KopeteContact>& KopeteProtocol::contacts()
{
	return m_contacts;
}

void KopeteProtocol::registerContact( KopeteContact *c )
{
	m_contacts.insert( c->contactId(), c );
	QObject::connect( c, SIGNAL( contactDestroyed( KopeteContact * ) ),
		SLOT( slotKopeteContactDestroyed( KopeteContact * ) ) );
}

void KopeteProtocol::slotKopeteContactDestroyed( KopeteContact *c )
{
	kdDebug() << "KopeteProtocol::slotKopeteContactDestroyed: " << c->contactId() << endl;
	m_contacts.remove( c->contactId() );
}

#include "kopeteprotocol.moc"

// vim: set noet ts=4 sts=4 sw=4:

