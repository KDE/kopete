/*
    kopetemessageevent.cpp - Kopete Message Event

    Copyright (c) 2002      by Duncan Mac-Vicar Prett       <duncan@kde.org>
    Copyright (c) 2002      by Hendrik vom Lehn        <hvl@linux-4-ever.de>
    Copyright (c) 2002-2003 by Martijn Klingens           <klingens@kde.org>
	Copyright (c) 2003      by Olivier Goffart      <ogoffart@kde.org>
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

#include "kopetemessageevent.h"

#include <kdebug.h>

#include "kopetemetacontact.h"
#include "kopetecontactlist.h"
#include "kopetecontact.h"
#include "kopetebehaviorsettings.h"

namespace Kopete
{

class MessageEvent::Private
{
public:
	Kopete::Message message;
	EventState state;
};

MessageEvent::MessageEvent( const Message& m, QObject *parent )
 : QObject(parent), d( new Private )
{
	d->message = m;
	d->state = Nothing;
	const Contact *c=m.from();
	if(c)
		connect(c,SIGNAL(contactDestroyed(Kopete::Contact*)),this,SLOT(discard()));
}

MessageEvent::~MessageEvent()
{
//	kDebug(14010) ;
	emit done(this);
	delete d;
}

Kopete::Message MessageEvent::message()
{
	return d->message;
}

void MessageEvent::setMessage( const Kopete::Message &message )
{
	d->message = message;
}

MessageEvent::EventState MessageEvent::state()
{
	return d->state;
}

void MessageEvent::apply()
{
	d->state = Applied;
	deleteLater();
}

void MessageEvent::ignore()
{
	// FIXME: this should be done by the contact list for itself.
	if( d->message.from()->metaContact() && d->message.from()->metaContact()->isTemporary() &&
		Kopete::BehaviorSettings::self()->balloonNotifyIgnoreClosesChatView() )
		ContactList::self()->removeMetaContact( d->message.from()->metaContact() );
	d->state = Ignored;
	deleteLater();
}

void MessageEvent::accept()
{
	emit accepted(this);
}

void MessageEvent::discard()
{
	emit discarded(this);
	delete this;
}

}

#include "kopetemessageevent.moc"

// vim: set noet ts=4 sts=4 sw=4:

