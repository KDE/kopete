/*
  aimcontactbase.cpp  -  AIM Contact Base

  Copyright (c) 2003 by Will Stephenson
  Copyright (c) 2006 by Roman Jarosz <kedgedev@centrum.cz>
  Kopete    (c) 2002-2006 by the Kopete developers  <kopete-devel@kde.org>

  *************************************************************************
  *                                                                       *
  * This program is free software; you can redistribute it and/or modify  *
  * it under the terms of the GNU General Public License as published by  *
  * the Free Software Foundation; either version 2 of the License, or     *
  * (at your option) any later version.                                   *
  *                                                                       *
  *************************************************************************
*/
#include "aimcontactbase.h"


#include "kopetechatsession.h"

#include "oscaraccount.h"

//liboscar
#include "oscarutils.h"

AIMContactBase::AIMContactBase( Kopete::Account* account, const QString& name, Kopete::MetaContact* parent,
                        const QString& icon )
: OscarContact(account, name, parent, icon )
{	
	m_mobile = false;
	// Set the last autoresponse time to the current time yesterday
	m_lastAutoresponseTime = QDateTime::currentDateTime().addDays(-1);
}

AIMContactBase::~AIMContactBase()
{
}

void AIMContactBase::sendAutoResponse(Kopete::Message& msg)
{
	// The target time is 2 minutes later than the last message
	int delta = m_lastAutoresponseTime.secsTo( QDateTime::currentDateTime() );
	kDebug(OSCAR_GEN_DEBUG) << "Last autoresponse time: " << m_lastAutoresponseTime;
	kDebug(OSCAR_GEN_DEBUG) << "Current time: " << QDateTime::currentDateTime();
	kDebug(OSCAR_GEN_DEBUG) << "Difference: " << delta;
	// Check to see if we're past that time
	if(delta > 120)
	{
		kDebug(OSCAR_GEN_DEBUG) << "Sending auto response";
		
		// This code was yoinked straight from OscarContact::slotSendMsg()
		// If only that slot wasn't private, but I'm not gonna change it right now.
		Oscar::Message message;
		
		if ( m_details.hasCap( CAP_UTF8 ) )
		{
			message.setText( Oscar::Message::UCS2, msg.plainBody() );
		}
		else
		{
			QTextCodec* codec = contactCodec();
			message.setText( Oscar::Message::UserDefined, msg.plainBody(), codec );
		}
		
		message.setTimestamp( msg.timestamp() );
		message.setSender( mAccount->accountId() );
		message.setReceiver( mName );
		message.setChannel( 0x01 );
		
		// isAuto defaults to false
		mAccount->engine()->sendMessage( message, true);
		kDebug(OSCAR_GEN_DEBUG) << "Sent auto response";
		manager(Kopete::Contact::CanCreate)->appendMessage(msg);
		manager(Kopete::Contact::CanCreate)->messageSucceeded();
		// Update the last autoresponse time
		m_lastAutoresponseTime = QDateTime::currentDateTime();
	}
	else
	{
		kDebug(OSCAR_GEN_DEBUG) << "Not enough time since last autoresponse, NOT sending";
	}
}

#include "aimcontactbase.moc"
//kate: tab-width 4; indent-mode csands;
