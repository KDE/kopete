/*
  Kopete Oscar Protocol
  profiletask.h - Update the user's profile on the server

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

#include "profiletask.h"

#include <qstring.h>
#include <kdebug.h>

#include "transfer.h"
#include "connection.h"
#include "oscartypes.h"
#include "oscarutils.h"

using namespace Oscar;

ProfileTask::ProfileTask( Task* parent )
		: Task( parent )
{
}


ProfileTask::~ProfileTask()
{
}


bool ProfileTask::forMe( const Transfer* transfer ) const
{
	Q_UNUSED( transfer );
	return false;
}

bool ProfileTask::take( Transfer* transfer )
{
	Q_UNUSED( transfer );
	return false;
}

void ProfileTask::onGo()
{
	sendProfileUpdate();
}

void ProfileTask::setProfileText( const QString& text )
{
	m_profileText = text;
}

void ProfileTask::setAwayMessage( const QString& text )
{
	m_awayMessage = text;
}

void ProfileTask::sendProfileUpdate()
{
	kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "SEND (CLI_SETUSERINFO/CLI_SET_LOCATION_INFO)" << endl;
	FLAP f = { 0x02, 0, 0 };
	SNAC s = { 0x0002, 0x0004, 0x0000, client()->snacSequence() };
	Buffer *buffer = new Buffer();
	Buffer capBuf;

	if ( !m_profileText.isNull() && !client()->isIcq() )
	{
		static const QString defencoding = "text/aolrtf; charset=\"us-ascii\"";
		buffer->addTLV(0x0001, defencoding.length(), defencoding.latin1());
		buffer->addTLV(0x0002, m_profileText.length(), m_profileText.local8Bit());
		kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "setting profile = " << m_profileText << endl;
	}

	if ( !m_awayMessage.isNull() && !client()->isIcq() )
	{
		static const QString defencoding = "text/aolrtf; charset=\"us-ascii\"";
		buffer->addTLV(0x0003, defencoding.length(), defencoding.latin1());
		buffer->addTLV(0x0004, m_awayMessage.length(), m_awayMessage.local8Bit());
		kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "setting away message = " << m_awayMessage << endl;
	}

	if ( client()->isIcq() )
	{
		capBuf.addString( oscar_caps[CAP_ICQSERVERRELAY], 16 ); // we support type-2 messages
		capBuf.addString( oscar_caps[CAP_UTF8], 16 ); // we can send/receive UTF encoded messages
		capBuf.addString( oscar_caps[CAP_ISICQ], 16 ); // I think this is an icq client, but maybe I'm wrong
		capBuf.addString( oscar_caps[CAP_KOPETE], 16 ); // we are the borg, resistance is futile
		//capBuf.addString( oscar_caps[CAP_RTFMSGS], 16 ); // we do incoming RTF messages
		capBuf.addString( oscar_caps[CAP_TYPING], 16 ); // we know you're typing something to us!
		capBuf.addString( oscar_caps[CAP_BUDDYICON], 16 ); //can you take my picture?
	}
	else
	{
		capBuf.addString( oscar_caps[CAP_UTF8], 16 ); //we can send/receive UTF encoded messages
		capBuf.addString( oscar_caps[CAP_KOPETE], 16 ); // we are the borg, resistance is futile
		capBuf.addString( oscar_caps[CAP_TYPING], 16 ); // we know you're typing something to us!
        capBuf.addString( oscar_caps[CAP_BUDDYICON], 16 ); //can you take my picture?
	}

	//kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "adding capabilities, size=" << capBuf.length() << endl;
	buffer->addTLV(0x0005, capBuf.length(), capBuf.buffer());
	Transfer* st = createTransfer( f, s , buffer );
	send( st );
	setSuccess( 0, QString::null );

}

//kate: tab-width 4; indent-mode csands;
