/*
  icqcontactbase.cpp  -  ICQ Contact Base

  Copyright (c) 2003      by Stefan Gehn  <metz@gehn.net>
  Copyright (c) 2003      by Olivier Goffart <oggoffart@kde.org>
  Copyright (c) 2006,2007 by Roman Jarosz <kedgedev@centrum.cz>
  Kopete    (c) 2003-2007 by the Kopete developers  <kopete-devel@kde.org>

  *************************************************************************
  *                                                                       *
  * This program is free software; you can redistribute it and/or modify  *
  * it under the terms of the GNU General Public License as published by  *
  * the Free Software Foundation; either version 2 of the License, or     *
  * (at your option) any later version.                                   *
  *                                                                       *
  *************************************************************************
*/

#include "icqcontactbase.h"

#include "kopetechatsessionmanager.h"

#include "oscaraccount.h"
#include "oscarutils.h"
#include "oscarpresence.h"
#include "oscarprotocol.h"
#include "oscarstatusmanager.h"


ICQContactBase::ICQContactBase( Kopete::Account *account, const QString &name, Kopete::MetaContact *parent,
						const QString& icon )
: OscarContact( account, name, parent, icon )
{
	QObject::connect( mAccount->engine(), SIGNAL(receivedXStatusMessage(QString,int,QString,QString)),
	                  this, SLOT(receivedXStatusMessage(QString,int,QString,QString)) );
}

ICQContactBase::~ICQContactBase()
{
}

void ICQContactBase::receivedXStatusMessage( const QString& contact, int icon, const QString& description, const QString& message )
{
	if ( Oscar::normalize( contact ) != Oscar::normalize( contactId() ) )
		return;

	OscarProtocol* p = static_cast<OscarProtocol *>(protocol());
	Oscar::Presence presence = p->statusManager()->presenceOf( this->onlineStatus() );
	presence.setFlags( presence.flags() | Oscar::Presence::XStatus );
	presence.setXtrazStatus( icon );
	setPresenceTarget( presence );

	Kopete::StatusMessage statusMessage;
	statusMessage.setTitle( description );
	statusMessage.setMessage( message );
	setStatusMessage( statusMessage );
}

#include "icqcontactbase.moc"
//kate: indent-mode csands; tab-width 4; replace-tabs off; space-indent off;
