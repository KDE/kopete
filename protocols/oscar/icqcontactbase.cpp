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
	QObject::connect( mAccount->engine(), SIGNAL(receivedXStatusMessage(const QString&, int, const QString&, const QString&)),
	                  this, SLOT(receivedXStatusMessage(const QString&, int, const QString&, const QString&)) );
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
	if ( !description.isEmpty() )
		setProperty( static_cast<OscarProtocol*>( protocol() )->statusTitle, description );
	else
		removeProperty( static_cast<OscarProtocol*>( protocol() )->statusTitle );

	setAwayMessage( message );
}

void ICQContactBase::slotSendMsg( Kopete::Message& msg, Kopete::ChatSession* session )
{
	//Why is this unused?
	Q_UNUSED( session );

	QTextCodec* codec = contactCodec();

	int messageChannel = 0x01;
	// Allow UCS2 because official AIM client doesn't sets the CAP_UTF8 anymore!
	bool allowUCS2 = !isOnline() || !(m_details.userClass() & Oscar::CLASS_ICQ) || m_details.hasCap( CAP_UTF8 );

	QString msgText( msg.plainBody() );
	// TODO: More intelligent handling of message length.
	const int chunk_length = 1274;
	int msgPosition = 0;

	do
	{
		QString msgChunk( msgText.mid( msgPosition, chunk_length ) );
		// Try to split on space if needed
		if ( msgChunk.length() == chunk_length )
		{
			for ( int i = 0; i < 100; i++ )
			{
				if ( msgChunk[chunk_length - i].isSpace() )
				{
					msgChunk = msgChunk.left( chunk_length - i );
					msgPosition++;
				}
			}
		}
		msgPosition += msgChunk.length();

		Oscar::Message message;
		message.setId( msg.id() );
		message.setText( Oscar::Message::encodingForText( msgChunk, allowUCS2 ), msgChunk, codec );
		message.setChannel( messageChannel );
		message.setTimestamp( msg.timestamp() );
		message.setSender( mAccount->accountId() );
		message.setReceiver( mName );
		mAccount->engine()->sendMessage( message );
	} while ( msgPosition < msgText.length() );

	msg.setState( Kopete::Message::StateSending );
	manager(Kopete::Contact::CanCreate)->appendMessage(msg);
	manager(Kopete::Contact::CanCreate)->messageSucceeded();
}

#include "icqcontactbase.moc"
//kate: indent-mode csands; tab-width 4; replace-tabs off; space-indent off;
