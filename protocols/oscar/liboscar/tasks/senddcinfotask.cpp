/*
   Kopete Oscar Protocol
   Send extended status info to the server

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
#include "senddcinfotask.h"

#include <kdebug.h>
#include "connection.h"
#include "buffer.h"
#include "oscarsettings.h"
#include "oscartypes.h"
#include "oscarutils.h"
#include "transfer.h"

SendDCInfoTask::SendDCInfoTask(Task* parent, Oscar::DWORD status): Task(parent), mStatus(status)
{
	mSendMood = false;
	mSendMessage = false;
	mMood = -1;
}


SendDCInfoTask::~SendDCInfoTask()
{
}

void SendDCInfoTask::setIcqMood( int mood )
{
	mMood = mood;
	mSendMood = true;
}

void SendDCInfoTask::setStatusMessage( const QString &message )
{
	mMessage = message;
	mSendMessage = true;
}

void SendDCInfoTask::onGo()
{
	FLAP f = { 0x02, 0, 0 };
	SNAC s = { 0x0001, 0x001E, 0x0000, client()->snacSequence() };
	Buffer* buffer = new Buffer();
	
	kDebug(OSCAR_RAW_DEBUG) << "Sending DC Info";

	/** \TODO Support something more than online in the status flags
	 *  \TODO Support something more than DC Disabled in the status flags
	 */
	/*
	if (status & ICQ_STATUS_SET_INVIS)
		sendChangeVisibility(0x03);
	else
		sendChangeVisibility(0x04);
	*/

	/* This is TLV 0x06 */
	//### Don't hardcode this value
	//Right now, it's always coded to not support DC
	Oscar::DWORD statusFlag = 0x01000000;
	if ( client()->settings()->webAware() )
	{
		kDebug(OSCAR_RAW_DEBUG) << "setting web aware on";
		statusFlag |= 0x00010000;
	}
	if ( client()->settings()->hideIP() )
	{
		kDebug(OSCAR_RAW_DEBUG) << "setting hide ip on";
		statusFlag |= 0x10000000;  // Direct connection upon authorization, hides IP
	}
	Buffer tlv06;
	tlv06.addDWord( statusFlag | mStatus );
	buffer->addTLV( 0x0006, tlv06.buffer() );

	/* Fill in the DC Info 
	 * We don't support Direct Connection yet. So fill in some
	 * dummy values
	 */
	Buffer tlv0C;
	tlv0C.addDWord( 0x00000000 );
	tlv0C.addWord( 0x0000 );
	tlv0C.addWord( 0x0000 );

	tlv0C.addByte( 0x00 ); // Mode, TODO: currently fixed to "Direct Connection disabled"
	tlv0C.addWord( ICQ_TCP_VERSION ); // icq tcp protocol version, v8 currently

	tlv0C.addDWord( 0x00000000 ); // Direct Connection Cookie
	tlv0C.addDWord( 0x00000050 ); // web front port
	tlv0C.addDWord( 0x00000003 ); // number of following client features
	tlv0C.addDWord( 0x00000000 ); // InfoUpdateTime
	tlv0C.addDWord( 0x00000000 ); // PhoneStatusTime
	tlv0C.addDWord( 0x00000000 ); // PhoneBookTime
	tlv0C.addWord( 0x0000 );
	buffer->addTLV( 0x000C, tlv0C.buffer() );

	buffer->addTLV16( 0x0008, 0x0A06 ); // we support online status messages

	if ( mSendMood || mSendMessage )
	{
		Buffer tlv1D;

		if ( mSendMessage )
		{
			Buffer tlv02;
			tlv02.addWord( 0x0002 );
			tlv02.addByte( 0x04 );
			QByteArray msgData = mMessage.toUtf8();
			msgData.truncate( 251 );
			tlv02.addByte( msgData.length() + 4 );
			tlv02.addWord( msgData.length() );
			tlv02.addString( msgData );
			tlv02.addWord( 0x0000 );
			tlv1D.addString( tlv02.buffer() );
		}

		if ( mSendMood )
		{
			QString mood = QString( "icqmood%1" ).arg( mMood );
			tlv1D.addTLV( 0x000E, mood.toLatin1() );
		}

		buffer->addTLV( 0x001D, tlv1D.buffer() );
	}

	Transfer* t = createTransfer( f, s, buffer );
	send( t );
	setSuccess( 0, QString() );
}

// kate: tab-width 4; indent-mode csands;
