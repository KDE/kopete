/*
    kopeteevent.cpp - Kopete Event

    Copyright (c) 2003      by Olivier Goffart <ogoffart@tiscalinet.be>
    Copyright (c) 2002      by Duncan Mac-Vicar Prett <duncan@kde.org>
    Copyright (c) 2002      by Hendrik vom Lehn <hvl@linux-4-ever.de>
    Copyright (c) 2002-2003 by Martijn Klingens       <klingens@kde.org>

    Kopete    (c) 2002-2003 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include <kdebug.h>

#include "kopeteevent.h"
#include "kopetemetacontact.h"
#include "kopetecontactlist.h"

KopeteEvent::KopeteEvent( const KopeteMessage& m, QObject *parent, const char *name ) : QObject(parent,name)
{
	m_message=m;
	m_state= Nothing;
}
KopeteEvent::~KopeteEvent()
{
	kdDebug(14010) << "KopeteEvent::~KopeteEvent " << endl;
	emit done(this);
}

KopeteMessage KopeteEvent::message()
{
	return m_message;
}

KopeteEvent::EventState KopeteEvent::state()
{
	return m_state;
}

void KopeteEvent::apply()
{
	kdDebug(14010) << "KopeteEvent::apply" << endl;
	m_state= Applied;
	deleteLater();
}

void KopeteEvent::ignore()
{
	if( m_message.from()->metaContact() && m_message.from()->metaContact()->isTemporary() )
		KopeteContactList::contactList()->removeMetaContact( m_message.from()->metaContact() );
	m_state= Ignored;
	deleteLater();
}

#include "kopeteevent.moc"

// vim: set noet ts=4 sts=4 sw=4:

