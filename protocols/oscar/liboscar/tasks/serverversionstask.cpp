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
				kDebug(OSCAR_RAW_DEBUG) << "Ignoring server versions";
				setSuccess( 0, QString() );
				setTransfer( 0 );
				return true;
				break;
			default:
				return false;
		}
	}
	return false;
}

QList<int> ServerVersionsTask::buildFamiliesList( Buffer* buffer )
{
        QList<int> familyList;

        kDebug(OSCAR_RAW_DEBUG) 
                                << "Got the list of families server supports"
                                << endl;

        if ( buffer->bytesAvailable() % 2 != 0 )
                return familyList;

        while ( buffer->bytesAvailable() != 0 )
                familyList.append( buffer->getWord() );

        return familyList;
}

void ServerVersionsTask::handleFamilies()
{
	QList<int> familyList = buildFamiliesList( transfer()->buffer() );
	client()->addToSupportedFamilies( familyList );
	requestFamilyVersions(); // send back a CLI_FAMILIES packet
}

void ServerVersionsTask::requestFamilyVersions()
{
	bool isIcq = client()->isIcq();
        QList<int> familiesList = client()->supportedFamilies();
	int listLength = familiesList.count();

	FLAP f = { 0x02, 0, 0 };
	SNAC s = { 0x0001, 0x0017, 0x0000, client()->snacSequence() };
	Oscar::WORD val;
	Buffer* outbuf = new Buffer();

	kDebug(OSCAR_RAW_DEBUG) << "SEND SNAC 0x01, 0x17 - Snac family versions we want";

	for ( int i = 0; i < listLength; i++ )
	{
		outbuf->addWord( familiesList[i] );
		if ( familiesList[i] == 0x0001 )
			val = 0x0004;
		else
		{
			if ( familiesList[i] == 0x0013 )
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
	send( st );
}

#include "serverversionstask.moc"
