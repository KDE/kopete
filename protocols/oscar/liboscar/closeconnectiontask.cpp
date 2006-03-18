/*
    Kopete Oscar Protocol
    closeconnectiontask.h - Handles the closing of the connection to the server

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

#include "closeconnectiontask.h"

#include <qstring.h>
#include <qvaluelist.h>
#include <kdebug.h>
#include <klocale.h>
#include "connection.h"
#include "transfer.h"
#include "oscarutils.h"

using namespace Oscar;

CloseConnectionTask::CloseConnectionTask( Task* parent )
	: Task(parent)
{
}


CloseConnectionTask::~CloseConnectionTask()
{
}

const QByteArray& CloseConnectionTask::cookie() const
{
	return m_cookie;
}

const QString& CloseConnectionTask::bosHost() const
{
	return m_bosHost;
}

const QString& CloseConnectionTask::bosPort() const
{
	return m_bosPort;
}

bool CloseConnectionTask::take( Transfer* transfer )
{
	QString errorReason;
	WORD errorNum = 0;
	if ( forMe( transfer ) )
	{
		kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "RECV (DISCONNECT)" << endl;

		FlapTransfer* ft = dynamic_cast<FlapTransfer*> ( transfer );
		
		if ( !ft )
		{
			kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo 
				<< "Could not convert transfer object to type FlapTransfer!!"  << endl;
			return false;
		}
		
		QValueList<TLV> tlvList = ft->buffer()->getTLVList();
			
		TLV uin = findTLV( tlvList, 0x0001 );
		if ( uin )
		{
			kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "found TLV(1) [UIN], uin=" << QString( uin.data ) << endl;
		}
	
		TLV err = findTLV( tlvList, 0x0008 );
		if ( !err )
			err = findTLV( tlvList, 0x0009 );

		if ( err.type == 0x0008 || err.type == 0x0009 )
		{
			errorNum = ( ( err.data[0] << 8 ) | err.data[1] );
	
			kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "found TLV(8) [ERROR] error= " << errorNum << endl;
	
			Oscar::SNAC s = { 0, 0, 0, 0 };
			client()->fatalTaskError( s, errorNum );
			return true; //if there's an error, we'll need to disconnect anyways
		}

		TLV server = findTLV( tlvList, 0x0005 );
		if ( server )
		{
			kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "found TLV(5) [SERVER] " << QString( server.data ) << endl;
			QString ip = server.data;
			int index = ip.find( ':' );
			m_bosHost = ip.left( index );
			ip.remove( 0 , index+1 ); //get rid of the colon and everything before it
			m_bosPort = ip;
		}

		TLV cookie = findTLV( tlvList, 0x0006 );
		if ( cookie )
		{
			kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "found TLV(6) [COOKIE]" << endl;
			m_cookie.duplicate( cookie.data );
		}
		
		tlvList.clear();
		
		if ( m_bosHost.isEmpty() )
		{
			kdWarning(OSCAR_RAW_DEBUG) << k_funcinfo << "Empty host address!" << endl;
			
			Oscar::SNAC s = { 0, 0, 0, 0 };
			client()->fatalTaskError( s, 0 );
			return true;
		}
		
		kdDebug( OSCAR_RAW_DEBUG ) << k_funcinfo << "We should reconnect to server '" 
			<< m_bosHost << "' on port " << m_bosPort << endl;
		setSuccess( errorNum, errorReason );
		return true;
	}
	return false;
}

bool CloseConnectionTask::forMe( const Transfer* transfer ) const
{
	const FlapTransfer* ft = dynamic_cast<const FlapTransfer*> ( transfer );

	if (!ft)
		return false;

	if ( ft && ft->flapChannel() == 4 )
		return true;
	else
		return false;
}

//kate: tab-width 4; indent-mode csands;
