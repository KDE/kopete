/*
   Kopete Oscar Protocol
   ssiparamstask.cpp - Get the SSI parameters so we can use them

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

#include "ssiparamstask.h"
#include <kdebug.h>
#include "buffer.h"
#include "connection.h"
#include "transfer.h"
#include "oscartypes.h"
#include "oscarutils.h"
#include "contactmanager.h"

using namespace Oscar;

SSIParamsTask::SSIParamsTask(Task* parent): Task(parent)
{
}


SSIParamsTask::~SSIParamsTask()
{
}


bool SSIParamsTask::forMe(const Transfer* transfer) const
{
	const SnacTransfer* st = dynamic_cast<const SnacTransfer*>( transfer );
	if ( !st )
		return false;

	if ( st->snacService() == 0x0013 && st->snacSubtype() == 0x0003 )
		return true;

	return false;
}

bool SSIParamsTask::take(Transfer* transfer)
{
	if ( forMe( transfer ) )
	{
		setTransfer( transfer );
		handleParamReply();
		setTransfer( 0 );
		return true;
	}

	return false;
}

void SSIParamsTask::onGo()
{
	FLAP f = { 0x02, 0, 0 };
	SNAC s = { 0x0013, 0x0002, 0x0000, client()->snacSequence() };

	Buffer* buffer = new Buffer();
	buffer->addTLV16( 0x000B, 0x000F );

	Transfer* t = createTransfer( f, s, buffer );
	send( t );
}

void SSIParamsTask::handleParamReply()
{
	kDebug( OSCAR_RAW_DEBUG ) << "Getting SSI parameters";
	Buffer* buf = transfer()->buffer();
	//manually parse the TLV out of the packet, since we only want certain things
	if ( buf->getWord() != 0x0004 )
	{
		setError( -1, QString() );
		return; //no TLV of type 0x0004, bad packet. do nothing.
	}
	else
	{
		buf->skipBytes( 2 ); //the tlv length
		Oscar::WORD maxContacts = buf->getWord();
		Oscar::WORD maxGroups = buf->getWord();
		Oscar::WORD maxVisible = buf->getWord();
		Oscar::WORD maxInvisible = buf->getWord();
		buf->skipBytes( 20 );
		Oscar::WORD maxIgnore = buf->getWord();
		client()->ssiManager()->setParameters( maxContacts, maxGroups, maxVisible, maxInvisible, maxIgnore );
	}
	setSuccess( 0, QString() );
}

// kate: tab-width 4; indent-mode csands;

