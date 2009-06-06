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
	m_sendCaps = false;
	m_xtrazStatus = -1;
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

void ProfileTask::setXtrazStatus( int xtrazStatus )
{
	if ( xtrazStatus < Oscar::XSTAT_LAST )
	{
		m_xtrazStatus = xtrazStatus;
		m_sendCaps = true;
	}
}

void ProfileTask::setCapabilities( bool value )
{
	m_sendCaps = value;
}

void ProfileTask::sendProfileUpdate()
{
	kDebug(OSCAR_RAW_DEBUG) << "SEND (CLI_SETUSERINFO/CLI_SET_LOCATION_INFO)";
	FLAP f = { 0x02, 0, 0 };
	SNAC s = { 0x0002, 0x0004, 0x0000, client()->snacSequence() };
	Buffer *buffer = new Buffer();

	if ( !m_profileText.isNull() )
	{
		static const QString defencoding = "text/aolrtf; charset=\"us-ascii\"";
		buffer->addTLV(0x0001, defencoding.toLatin1());
		buffer->addTLV(0x0002, m_profileText.toLocal8Bit());
		kDebug(OSCAR_RAW_DEBUG) << "setting profile = " << m_profileText;
	}

	if ( !m_awayMessage.isNull() )
	{
		static const QString defencoding = "text/aolrtf; charset=\"us-ascii\"";
		buffer->addTLV(0x0003, defencoding.toLatin1());
		buffer->addTLV(0x0004, m_awayMessage.toLocal8Bit());
		kDebug(OSCAR_RAW_DEBUG) << "setting away message = " << m_awayMessage;
	}

	if ( m_sendCaps )
	{
		Buffer capBuf;
		if ( client()->isIcq() )
		{
			capBuf.addGuid( oscar_caps[CAP_ICQSERVERRELAY] ); // we support type-2 messages
			capBuf.addGuid( oscar_caps[CAP_DIRECT_ICQ_COMMUNICATION] ); // we support direct communication
			capBuf.addGuid( oscar_caps[CAP_XTRAZ] ); // we support xtraz

			if ( m_xtrazStatus > -1 )
				capBuf.addGuid( oscar_xStatus[m_xtrazStatus] ); // set xtraz status
		}
		capBuf.addGuid( oscar_caps[CAP_NEWCAPS] ); // we understand the new format of caps (xtraz status)
		capBuf.addGuid( oscar_caps[CAP_SENDFILE] ); // we can do filetransfers! :)
		capBuf.addGuid( oscar_caps[CAP_UTF8] ); // we can send/receive UTF encoded messages
		// send version
		capBuf.addGuid( client()->versionCap() );
		capBuf.addGuid( oscar_caps[CAP_TYPING] ); // we know you're typing something to us!
		capBuf.addGuid( oscar_caps[CAP_BUDDYICON] ); //can you take my picture?
		capBuf.addGuid( oscar_caps[CAP_INTEROPERATE] ); //AIM can communicate with ICQ users and ICQ with AIM users.

		capBuf.addGuid( oscar_caps[CAP_CHAT] );

		kDebug(OSCAR_RAW_DEBUG) << "adding capabilities, size=" << capBuf.length();
		buffer->addTLV(0x0005, capBuf.buffer());
	}

	Transfer* st = createTransfer( f, s , buffer );
	send( st );
	setSuccess( 0, QString() );
	kDebug(OSCAR_RAW_DEBUG) << "done.";
}

//kate: tab-width 4; indent-mode csands;
