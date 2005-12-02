/*
   Kopete Oscar Protocol
   fakelocalcontactlisttask.cpp

   Copyright (c) 2005 Jan Ritzerfeld <kde@bugs.jan.ritzerfeld.net>
   Copyright (c) 2004 Matt Rogers <mattr@kde.org>

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
#include "localcontactlisttask.h"

#include <kdebug.h>
#include "buffer.h"
#include "connection.h"
#include "ssimanager.h"
#include "oscarsettings.h"
#include "oscartypes.h"
#include "oscarutils.h"
#include "transfer.h"
//Added by qt3to4:
#include <Q3ValueList>

using namespace Oscar;

#define PACKET_SIZE_LIMIT 8000

LocalContactListTask::LocalContactListTask(Task* parent): Task(parent)
{
}


LocalContactListTask::~LocalContactListTask()
{
}


void LocalContactListTask::onGo()
{
	if( !client()->settings()->respectRequireAuth() )
	{
		FLAP f = { 0x02, 0, 0 };
		SNAC s = { 0x0003, 0x0004, 0x0000, client()->snacSequence() };
		Buffer* buffer = new Buffer();
		
		kdDebug( OSCAR_RAW_DEBUG ) << k_funcinfo << "Sending contact list" << endl;
		Q3ValueList<Oscar::SSI> contactList = client()->ssiManager()->contactList();
		Q3ValueList<Oscar::SSI>::const_iterator cEnd = contactList.constEnd();
		for ( Q3ValueList<Oscar::SSI>::const_iterator it = contactList.constBegin(); it != cEnd; ++it )
		{
			if ( ( buffer->length() + ( *it ).name().length() ) 
				< PACKET_SIZE_LIMIT )
			{
				kdDebug( OSCAR_RAW_DEBUG ) << "Adding contact " << ( *it ).name() << " to CLI_BUDDYLIST_ADD packet" << endl;
				buffer->addBUIN( ( *it ).name().latin1() );
			}
			else
			{
				kdDebug( OSCAR_RAW_DEBUG ) << "CLI_BUDDYLIST_ADD packet is full. Transmitting the packet" << endl;
				Transfer* t = createTransfer( f, s, buffer );
				send( t );

				buffer = new Buffer();
				kdDebug( OSCAR_RAW_DEBUG ) << "Adding contact " << ( *it ).name() << " to CLI_BUDDYLIST_ADD packet" << endl;
				buffer->addBUIN( ( *it ).name().latin1() );
			}
		}
	
		Transfer* t = createTransfer( f, s, buffer );
		send( t );
	}
	
	setSuccess( 0, QString::null );
}

//kate: tab-width 4; indent-mode csands;
