/*
    Kopete Oscar Protocol
    serverversionstask.cpp - Handles the snac family versions

    Copyright (c) 2004 Matt Rogers <mattr@kde.org>

    Kopete (c) 2002-2004 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "serverversionstask.h"

#include <kdebug.h>

#include "connection.h"
#include "buffer.h"
#include "oscartypes.h"
#include "oscarutils.h"
#include "transfer.h"


using namespace Oscar;

ServerVersionsTask::ServerVersionsTask( Task* parent )
 : Task( parent )
{
    m_family = 0;
}


ServerVersionsTask::~ServerVersionsTask()
{
}


bool ServerVersionsTask::forMe( const Transfer* transfer ) const
{
	const SnacTransfer* st = dynamic_cast<const SnacTransfer*> ( transfer );
	
	if (!st)
		return false;
	
	if ( st->snacService() == 1 )
	{
		switch ( st->snacSubtype() )
		{
		case 0x03:
		case 0x17:
		case 0x18:
			return true;
			break;
		default:
			return false;
		}
	}
	return false;
}

bool ServerVersionsTask::take( Transfer* transfer )
{	
	SnacTransfer* st = dynamic_cast<SnacTransfer*> ( transfer );
	if (!st)
		return false;
	
	if ( forMe( transfer ) )
	{
		switch ( st->snacSubtype() )
		{
			case 0x03:
				setTransfer( transfer );
				handleFamilies();
				setTransfer( 0 );
				return true;
				break;
			case 0x18:
				setTransfer( transfer );
				handleServerVersions();
				setTransfer( 0 );
				return true;
				break;
			default:
				return false;
		}
	}
	return false;
}

void ServerVersionsTask::handleFamilies()
{
	kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo 
		<< "RECV SNAC 0x01, 0x03 - got the list of families server supports" << endl;

	Buffer* outbuf = transfer()->buffer();
	if ( outbuf->length() % 2 != 0 )
	{
		setError( -1, QString::null );
		return;
	}

	while ( outbuf->length () != 0 )
	{
		m_familiesList.append( outbuf->getWord() );
	}
	client()->addToSupportedFamilies( m_familiesList );
	requestFamilyVersions(); // send back a CLI_FAMILIES packet
}

void ServerVersionsTask::requestFamilyVersions()
{
	bool isIcq = client()->isIcq();
	int listLength = m_familiesList.count();

	FLAP f = { 0x02, 0, 0 };
	SNAC s = { 0x0001, 0x0017, 0x0000, client()->snacSequence() };
	WORD val;
	Buffer* outbuf = new Buffer();

	kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "SEND SNAC 0x01, 0x17 - Snac family versions we want" << endl;

	for ( int i = 0; i < listLength; i++ )
	{
		outbuf->addWord( m_familiesList[i] );
		if ( m_familiesList[i] == 0x0001 )
			val = 0x0003;
		else
		{
			if ( m_familiesList[i] == 0x0013 )
			{
				if ( isIcq )
					val = 0x0004; // for ICQ2002
				else
					val = 0x0003;
			}
			else
				val = 0x0001;
		}

		outbuf->addWord(val);
	}

	Transfer* st = createTransfer( f, s, outbuf );
	st->toString();
	send( st );
}

void ServerVersionsTask::handleServerVersions()
{
	kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo <<
		"RECV SNAC 0x01, 0x18, got list of families this server understands" << endl;

	Buffer* buffer = transfer()->buffer();
	int numFamilies = m_familiesList.count();
	for ( int srvFamCount = 0; srvFamCount < numFamilies; srvFamCount++ )
	{
		kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "server version=" << buffer->getWord()
			 << ", server family=" << buffer->getWord() << endl;
	}
	setSuccess( 0, QString::null );
}

#include "serverversionstask.moc"
