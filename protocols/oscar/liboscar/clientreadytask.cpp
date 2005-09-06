/*
   Kopete Oscar Protocol
   $FILENAME.cpp

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
#include "clientreadytask.h"

#include <kdebug.h>
#include "buffer.h"
#include "connection.h"
#include "rateclass.h"
#include "rateclassmanager.h"
#include "oscartypes.h"
#include "oscarutils.h"
#include "transfer.h"

using namespace Oscar;

ClientReadyTask::ClientReadyTask(Task* parent): Task(parent)
{
	m_classList = client()->rateManager()->classList();
}


ClientReadyTask::~ClientReadyTask()
{
}

void ClientReadyTask::setFamilies( const QValueList<int>& families )
{
	m_familyList = families;
}


void ClientReadyTask::onGo()
{
	FLAP f = { 0x02, 0, 0 };
	SNAC s = { 0x0001, 0x0002, 0x0000, client()->snacSequence() };
	Buffer* buffer = new Buffer();
	
	kdDebug( OSCAR_RAW_DEBUG ) << k_funcinfo << "Sending client ready, end of login" << endl;
	//nasty nasty nasty hack to get all the packets to work
	QValueList<int>::const_iterator rcEnd = m_familyList.constEnd();
	for ( QValueList<int>::const_iterator it = m_familyList.constBegin(); it != rcEnd; ++it )
	{
		//I have no idea what any of these values mean. I just copied them from oscarsocket
		int i = ( *it );
		buffer->addWord( i );
		switch ( i )
		{
		case 0x0001:
			buffer->addWord( 0x0003 );
			break;
		case 0x0013:
			buffer->addWord( client()->isIcq() ? 0x0002 : 0x0003 );
			break;
		default:
			buffer->addWord( 0x0001 );
		};
		
		if ( client()->isIcq() )
		{
			if ( i == 0x0002 )
				buffer->addWord( 0x0101 );
			else
				buffer->addWord( 0x0110 );
			
			//always add 0x047B
			buffer->addWord( 0x047B );
		}
		else //we're AIM so AOL has us do something completely different! *sigh*
		{
			switch( i )
			{
			case 0x0008:
			case 0x000B:
			case 0x000C:
				buffer->addWord( 0x0104 );
				buffer->addWord( 0x0001 );
				break;
			default:
				buffer->addWord( 0x0110 );
				buffer->addWord( 0x059B );
				break;
			};
		}
	}

	//send the damn thing so we can finally be finished
	//with the hell that is oscar login. (just wait until you get a message)
	Transfer* t = createTransfer( f, s, buffer );
	send( t );
	setSuccess( 0, QString::null );

}

//kate: tab-width 4; indent-mode csands;
