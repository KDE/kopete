/*
    kopetemessageevent.cpp - Kopete Message Event

    Copyright (c) 2003      by Olivier Goffart      <ogoffart@tiscalinet.be>
    Copyright (c) 2002      by Duncan Mac-Vicar Prett       <duncan@kde.org>
    Copyright (c) 2002      by Hendrik vom Lehn        <hvl@linux-4-ever.de>
    Copyright (c) 2002-2003 by Martijn Klingens           <klingens@kde.org>
    Copyright (c) 2004      by Richard Smith         <richard@metafoo.co.uk>

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

#include "kopetemessageevent.h"
#include "kopetemetacontact.h"
#include "kopetecontactlist.h"

namespace Kopete
{

MessageEvent::MessageEvent( const Message& m, QObject *parent, const char *name ) : QObject(parent,name)
{
	m_message=m;
	m_state= Nothing;
}

MessageEvent::~MessageEvent()
{
	kdDebug(14010) << k_funcinfo << endl;
	emit done(this);
}

Kopete::Message MessageEvent::message()
{
	return m_message;
}

MessageEvent::EventState MessageEvent::state()
{
	return m_state;
}

void MessageEvent::apply()
{
	kdDebug(14010) << k_funcinfo << endl;
	m_state= Applied;
	deleteLater();
}

void MessageEvent::ignore()
{
	if( m_message.from()->metaContact() && m_message.from()->metaContact()->isTemporary() )
		ContactList::contactList()->removeMetaContact( m_message.from()->metaContact() );
	m_state= Ignored;
	deleteLater();
}

}

#include "kopetemessageevent.moc"

// vim: set noet ts=4 sts=4 sw=4:

