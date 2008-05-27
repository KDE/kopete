/*
	Kopete Oscar Protocol
	warningtask.cpp - send warnings to aim users

	Copyright (c) 2005 by Matt Rogers <mattr@kde.org>

	Kopete (c) 2002-2005 by the Kopete developers <kopete-devel@kde.org>

	*************************************************************************
	*                                                                       *
	* This library is free software; you can redistribute it and/or         *
	* modify it under the terms of the GNU Lesser General Public            *
	* License as published by the Free Software Foundation; either          *
	* version 2 of the License, or (at your option) any later version.      *
	*                                                                       *
	*************************************************************************
*/

#include "warningtask.h"

#include <qstring.h>
#include <kdebug.h>
#include "transfer.h"
#include "connection.h"

WarningTask::WarningTask( Task* parent ): Task( parent )
{
}


WarningTask::~WarningTask()
{
}

void WarningTask::setContact( const QString& contact )
{
	m_contact = contact;
}

void WarningTask::setAnonymous( bool anon )
{
	m_sendAnon = anon;
}

bool WarningTask::forMe( const Transfer* transfer ) const
{
	const SnacTransfer* st = dynamic_cast<const SnacTransfer*>( transfer );
	if ( !st )
		return false;
	if ( st->snacService() == 0x04 && st->snacSubtype() == 0x09 && st->snacRequest() == m_sequence )
		return true;
	
	return false;
}

bool WarningTask::take( Transfer* transfer )
{
	if ( forMe( transfer ) )
	{
		setTransfer( transfer );
		Buffer *b = transfer->buffer();
		m_increase = b->getWord();
		m_newLevel = b->getWord();
		kDebug(OSCAR_RAW_DEBUG) << "Got warning ack for " << m_contact;
		kDebug(OSCAR_RAW_DEBUG) << "Warning level increased " << m_increase 
			<< " to " << m_newLevel << endl;
		emit userWarned( m_contact, m_increase, m_newLevel );
		setSuccess( 0, QString() );
		setTransfer( 0 );
		return true;
	}
	else
	{
		setError( 0, QString() );
		return false;
	}
}

void WarningTask::onGo()
{
	FLAP f = { 0x0002, 0, 0 };
	SNAC s = { 0x0004, 0x0008, 0x0000, client()->snacSequence() };
	Buffer* b = new Buffer;
	if ( m_sendAnon )
		b->addWord( 0x0001 );
	else
		b->addWord( 0x0000 );
	
	b->addBUIN( m_contact.toLatin1() ); //TODO i should probably check the encoding here. nyeh
	Transfer* t = createTransfer( f, s, b );
	send( t );
}

//kate: indent-mode csands; space-indent off; replace-tabs off; tab-width 4;

#include "warningtask.moc"
