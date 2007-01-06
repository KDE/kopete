/*
  icqcontactbase.cpp  -  ICQ Contact Base

  Copyright (c) 2003      by Stefan Gehn  <metz AT gehn.net>
  Copyright (c) 2003      by Olivier Goffart
  Copyright (c) 2006      by Roman Jarosz <kedgedev@centrum.cz>
  Kopete    (c) 2003-2006 by the Kopete developers  <kopete-devel@kde.org>

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


ICQContactBase::ICQContactBase( Kopete::Account *account, const QString &name, Kopete::MetaContact *parent,
						const QString& icon, const OContact& ssiItem )
: OscarContact( account, name, parent, icon, ssiItem )
{
	m_requestingNickname = false;

	QObject::connect( mAccount->engine(), SIGNAL(receivedIcqShortInfo(const QString&)),
	                  this, SLOT(receivedShortInfo(const QString&)) );
	QObject::connect( mAccount->engine(), SIGNAL(receivedXStatusMessage(const QString&, int, const QString&, const QString&)),
	                  this, SLOT(receivedXStatusMessage(const QString&, int, const QString&, const QString&)) );
}

ICQContactBase::~ICQContactBase()
{
}

QString ICQContactBase::sanitizedMessage( const QString& message )
{
	return message;
}

void ICQContactBase::requestShortInfo()
{
	if ( mAccount->isConnected() )
		mAccount->engine()->requestShortInfo( contactId() );
}

void ICQContactBase::receivedShortInfo( const QString& contact )
{
	if ( Oscar::normalize( contact ) != Oscar::normalize( contactId() ) )
		return;

	QTextCodec* codec = contactCodec();

	m_requestingNickname = false; //done requesting nickname
	ICQShortInfo shortInfo = mAccount->engine()->getShortInfo( contact );
	/*
	if(!shortInfo.firstName.isEmpty())
		setProperty( mProtocol->firstName, codec->toUnicode( shortInfo.firstName ) );
	else
		removeProperty(mProtocol->firstName);

	if(!shortInfo.lastName.isEmpty())
		setProperty( mProtocol->lastName, codec->toUnicode( shortInfo.lastName ) );
	else
		removeProperty(mProtocol->lastName);
	*/
	if ( m_ssiItem.alias().isEmpty() && !shortInfo.nickname.isEmpty() )
	{
		kDebug(OSCAR_GEN_DEBUG) << k_funcinfo <<
			"setting new displayname for former UIN-only Contact" << endl;
		setProperty( Kopete::Global::Properties::self()->nickName(), codec->toUnicode( shortInfo.nickname ) );
	}
}

void ICQContactBase::receivedXStatusMessage( const QString& contact, int icon, const QString& title, const QString& desc )
{
	if ( Oscar::normalize( contact ) != Oscar::normalize( contactId() ) )
		return;

	// TODOL create OnlineStatus with icon and title
	setAwayMessage( desc );
	
	m_haveAwayMessage = true;
}

void ICQContactBase::slotSendMsg( Kopete::Message& msg, Kopete::ChatSession* session )
{
	//Why is this unused?
	Q_UNUSED( session );

	QTextCodec* codec = contactCodec();

	int messageChannel = 0x01;
	Oscar::Message::Encoding messageEncoding;

	if ( isOnline() && m_details.hasCap( CAP_UTF8 ) )
		messageEncoding = Oscar::Message::UCS2;
	else
		messageEncoding = Oscar::Message::UserDefined;

	QString msgText( msg.plainBody() );
	// TODO: More intelligent handling of message length.
	uint chunk_length = !isOnline() ? 450 : 4096;
	uint msgPosition = 0;

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

		Oscar::Message message( messageEncoding, msgChunk, messageChannel, 0, msg.timestamp(), codec );
		message.setSender( mAccount->accountId() );
		message.setReceiver( mName );
		mAccount->engine()->sendMessage( message );
	} while ( msgPosition < msgText.length() );

	manager(Kopete::Contact::CanCreate)->appendMessage(msg);
	manager(Kopete::Contact::CanCreate)->messageSucceeded();
}

#include "icqcontactbase.moc"
//kate: indent-mode csands; tab-width 4; replace-tabs off; space-indent off;
