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
	kDebug(OSCAR_RAW_DEBUG) << "Sending ICBM Parameters request";
	FLAP f = { 0x02, 0, 0 };
	SNAC s = { 0x0004, 0x0004, 0x0000, client()->snacSequence() };
	Buffer* buffer = new Buffer();
	Transfer* st = createTransfer( f, s, buffer );
	send( st );
}

void ICBMParamsTask::handleICBMParameters()
{
	Buffer* buffer = transfer()->buffer();
	
	Oscar::WORD channel = buffer->getWord();
	kDebug(OSCAR_RAW_DEBUG) << "channel=" << channel;
	
	/**
	 * bit1: messages allowed for specified channel
	 * bit2: missed calls notifications enabled for specified channel
	 * bit4: client supports typing notifications
	 */
	Oscar::DWORD messageFlags = buffer->getDWord();
	Oscar::WORD maxMessageSnacSize = buffer->getWord();
	Oscar::WORD maxSendWarnLvl = buffer->getWord(); // max sender Warning Level
	Oscar::WORD maxRecvWarnLvl = buffer->getWord(); // max Receiver Warning Level
	Oscar::WORD minMsgInterval = buffer->getWord(); // minimum message interval (msec)
	
	
	kDebug(OSCAR_RAW_DEBUG) << "messageFlags       = " << messageFlags;
	kDebug(OSCAR_RAW_DEBUG) << "maxMessageSnacSize = " << maxMessageSnacSize;
	kDebug(OSCAR_RAW_DEBUG) << "maxSendWarnLvl     = " << maxSendWarnLvl;
	kDebug(OSCAR_RAW_DEBUG) << "maxRecvWarnLvl     = " << maxRecvWarnLvl;
	kDebug(OSCAR_RAW_DEBUG) << "minMsgInterval     = " << minMsgInterval;
	
	/*WORD unknown = */buffer->getWord();

	// Now we set our own parameters.
	// The ICBM parameters have to be set up separately for each channel.
	// Some clients (namely Trillian) might refuse sending on channels that were not set up.
	sendMessageParams( 0x01 );
	sendMessageParams( 0x02 );
	sendMessageParams( 0x04 );
}

void ICBMParamsTask::sendMessageParams( int channel )
{
	kDebug(OSCAR_RAW_DEBUG) << "Sending ICBM parameters for channel " << channel;
	
	FLAP f = { 0x02, 0, 0 };
	SNAC s = { 0x0004, 0x0002, 0x0000, client()->snacSequence() };
	Buffer* buffer = new Buffer();
		
	// the channel for which to set up the parameters
	buffer->addWord( channel );
	
	const Oscar::DWORD OFFLINE_MESSAGES = 0x00000100;
	const Oscar::DWORD RTF_MESSAGES = 0x00000600; // Maybe something more

	//these are all read-write
	// channel-flags
	// bit 1 : messages allowed (always 1 or you cannot send IMs)
	// bit 2 : missed call notifications enabled
	// bit 4 : typing notifications enabled
	if ( channel == 1 )
		buffer->addDWord(0x0000000B | OFFLINE_MESSAGES | RTF_MESSAGES);
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
	setSuccess( 0, QString() );
}
//kate: tab-width 4; indent-mode csands;

