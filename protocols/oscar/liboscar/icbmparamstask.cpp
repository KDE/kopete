/*
	Kopete Oscar Protocol
	icbmparamstask.cpp - Get the ICBM parameters
	
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
#include "icbmparamstask.h"

#include <kdebug.h>
#include "connection.h"
#include "oscartypes.h"
#include "transfer.h"
#include "oscarutils.h"
#include "buffer.h"

ICBMParamsTask::ICBMParamsTask( Task* parent )
		: Task( parent )
{}


ICBMParamsTask::~ICBMParamsTask()
{}


bool ICBMParamsTask::forMe( const Transfer* transfer ) const
{
	const SnacTransfer* st = dynamic_cast<const SnacTransfer*>( transfer );
	
	if (!st)
		return false;
	
	if ( st->snacService() == 4 && st->snacSubtype() == 5 )
		return true;
	else
		return false;
}

bool ICBMParamsTask::take( Transfer* transfer )
{
	if ( forMe( transfer ) )
	{
		setTransfer( transfer );
		handleICBMParameters();
		setTransfer( 0 );
		return true;
	}
	return false;
}

void ICBMParamsTask::onGo()
{
	kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Sending ICBM Parameters request" << endl;
	FLAP f = { 0x02, 0, 0 };
	SNAC s = { 0x0004, 0x0004, 0x0000, client()->snacSequence() };
	Buffer* buffer = new Buffer();
	Transfer* st = createTransfer( f, s, buffer );
	send( st );
}

void ICBMParamsTask::handleICBMParameters()
{
	Buffer* buffer = transfer()->buffer();
	
	WORD channel = buffer->getWord();
	kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "channel=" << channel << endl;
	
	/**
	 * bit1: messages allowed for specified channel
	 * bit2: missed calls notifications enabled for specified channel
	 * bit4: client supports typing notifications
	 */
	DWORD messageFlags = buffer->getDWord();
	WORD maxMessageSnacSize = buffer->getWord();
	WORD maxSendWarnLvl = buffer->getWord(); // max sender Warning Level
	WORD maxRecvWarnLvl = buffer->getWord(); // max Receiver Warning Level
	WORD minMsgInterval = buffer->getWord(); // minimum message interval (msec)
	
	
	kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "messageFlags       = " << messageFlags << endl;
	kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "maxMessageSnacSize = " << maxMessageSnacSize << endl;
	kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "maxSendWarnLvl     = " << maxSendWarnLvl << endl;
	kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "maxRecvWarnLvl     = " << maxRecvWarnLvl << endl;
	kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "minMsgInterval     = " << minMsgInterval << endl;
	
	/*WORD unknown = */buffer->getWord();

	// Now we set our own parameters.
	// The ICBM parameters have to be set up seperately for each channel.
	// Some clients (namely Trillian) might refuse sending on channels that were not set up.
	sendMessageParams( 0x01 );
	sendMessageParams( 0x02 );
	sendMessageParams( 0x04 );
}

void ICBMParamsTask::sendMessageParams( int channel )
{
	kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Sending ICBM parameters for channel " << channel << endl;
	
	FLAP f = { 0x02, 0, 0 };
	SNAC s = { 0x0004, 0x0002, 0x0000, client()->snacSequence() };
	Buffer* buffer = new Buffer();
		
	// the channel for which to set up the parameters
	buffer->addWord( channel );
	
	//these are all read-write
	// channel-flags
	// bit 1 : messages allowed (always 1 or you cannot send IMs)
	// bit 2 : missed call notifications enabled
	// bit 4 : typing notifications enabled
	if ( channel == 1 )
		buffer->addDWord(0x0000000B);
	else
		buffer->addDWord(0x00000003);
	
	//max message length (8000 bytes)
	buffer->addWord(0x1f40);
	//max sender warning level (999)
	buffer->addWord(0x03e7);
	//max receiver warning level  (999)
	buffer->addWord(0x03e7);
	//min message interval limit  (0 msec)
	buffer->addWord(0x0000);
	// unknown parameter
	buffer->addWord(0x0000);
	
	Transfer* st = createTransfer( f, s, buffer );
	send( st );
	setSuccess( 0, QString::null );
}
//kate: tab-width 4; indent-mode csands;

