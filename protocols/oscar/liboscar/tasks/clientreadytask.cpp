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
#include <QList>

using namespace Oscar;

ClientReadyTask::ClientReadyTask(Task* parent): Task(parent)
{
	m_classList = client()->rateManager()->classList();
}


ClientReadyTask::~ClientReadyTask()
{
}

void ClientReadyTask::setFamilies( const QList<int>& families )
{
	m_familyList = families;
}


void ClientReadyTask::onGo()
{
	FLAP f = { 0x02, 0, 0 };
	SNAC s = { 0x0001, 0x0002, 0x0000, client()->snacSequence() };
	Buffer* buffer = new Buffer();
	
	kDebug( OSCAR_RAW_DEBUG ) << "Sending client ready, end of login";
	//nasty nasty nasty hack to get all the packets to work
	QList<int>::const_iterator rcEnd = m_familyList.constEnd();
	for ( QList<int>::const_iterator it = m_familyList.constBegin(); it != rcEnd; ++it )
	{
		//I have no idea what any of these values mean. I just copied them from oscarsocket
		int i = ( *it );
		buffer->addWord( i );
		switch ( i )
		{
		case 0x0001:
			buffer->addWord( 0x0004 );
			break;
		case 0x0013:
			buffer->addWord( client()->isIcq() ? 0x0004 : 0x0003 );
			break;
		default:
			buffer->addWord( 0x0001 );
		};
		
		if ( client()->isIcq() )
		{
			buffer->addDWord( 0x0110164F ); // ICQ dll library version
		}
		else
		{
			buffer->addDWord( 0x0110145D ); // AIM dll library version
		}
	}

	//send the damn thing so we can finally be finished
	//with the hell that is oscar login. (just wait until you get a message)
	Transfer* t = createTransfer( f, s, buffer );
	send( t );
	setSuccess( 0, QString() );

}

//kate: tab-width 4; indent-mode csands;
