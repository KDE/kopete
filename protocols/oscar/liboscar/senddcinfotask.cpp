/*
   Kopete Oscar Protocol
   senddcinfotask.cpp - Send the DC info to the server

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

SendDCInfoTask::SendDCInfoTask(Task* parent, DWORD status): Task(parent), mStatus(status)
{
}


SendDCInfoTask::~SendDCInfoTask()
{
}


void SendDCInfoTask::onGo()
{
	FLAP f = { 0x02, 0, 0 };
	SNAC s = { 0x0001, 0x001E, 0x0000, client()->snacSequence() };
	Buffer* buffer = new Buffer();
	
	kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Sending DC Info" << endl;

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
	buffer->addWord( 0x0006 );
	buffer->addWord( 0x0004 );
	//### Don't hardcode this value
	//Right now, it's always coded to not support DC
	DWORD statusFlag = 0x01000000;
	if ( client()->settings()->webAware() )
	{
		kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "setting web aware on" << endl;
		statusFlag |= 0x00010000;
	}
	if ( client()->settings()->hideIP() )
	{
		kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "setting hide ip on" << endl;
		statusFlag |= 0x10000000;  // Direct connection upon authorization, hides IP
	}
	
	buffer->addDWord( statusFlag | mStatus );

	/* Fill in the DC Info 
	 * We don't support Direct Connection yet. So fill in some
	 * dummy values
	 */
	buffer->addWord( 0x000C ); //TLV Type 0x0C
	buffer->addWord( 0x0025 );

	buffer->addDWord( 0x00000000 );
	buffer->addWord( 0x0000 );
	buffer->addWord( 0x0000 );

	buffer->addByte( 0x00 ); // Mode, TODO: currently fixed to "Direct Connection disabled"
	buffer->addWord( ICQ_TCP_VERSION ); // icq tcp protocol version, v8 currently

	buffer->addDWord( 0x00000000 ); // Direct Connection Cookie
	buffer->addDWord( 0x00000050 ); // web front port
	buffer->addDWord( 0x00000003 ); // number of following client features
	buffer->addDWord( 0x00000000 ); // InfoUpdateTime
	buffer->addDWord( 0x00000000 ); // PhoneStatusTime
	buffer->addDWord( 0x00000000 ); // PhoneBookTime
	buffer->addWord( 0x0000 );

	buffer->addWord( 0x0008 ); // TLV(8)
	buffer->addWord( 0x0002 ); // length 2
	buffer->addWord( 0x0000 ); // error code - 0

	Transfer* t = createTransfer( f, s, buffer );
	send( t );
	setSuccess( 0, QString::null );
}


// kate: tab-width 4; indent-mode csands;
