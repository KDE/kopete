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
	
			QString errorReason;
			bool needDisconnect = parseDisconnectCode(errorNum, errorReason);
			if ( needDisconnect )
			{
			  emit disconnected( errorNum, errorReason );
			  return true; //if there's an error, we'll need to disconnect anyways
			}
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

bool CloseConnectionTask::parseDisconnectCode( int error, QString& reason )
{
	QString acctType = ( client()->isIcq() ? i18n("ICQ") : i18n("AIM"));
	QString acctDescription = ( client()->isIcq() ? "UIN" : "screen name");

	switch ( error )
	{
	case 0x0001:
	{
		if ( client()->isLoggedIn() ) // multiple logins (on same UIN)
		{
			reason = i18n( "You have logged in more than once with the same %1," \
				" account %2 is now disconnected.")
				.arg( acctDescription ).arg( client()->userId() );
		}
		else // error while logging in
		{
			reason = i18n( "Sign on failed because either your %1 or " \
				"password are invalid. Please check your settings for account %2.")
				.arg( acctDescription ).arg( client()->userId() );
			return true;
		}
		break;
	}

	case 0x0002: // Service temporarily unavailable
	case 0x0014: // Reservation map error
	{
		reason = i18n("The %1 service is temporarily unavailable. Please try again later.")
			.arg( acctType );
		break;
	}

	case 0x0004: // Incorrect nick or password, re-enter
	case 0x0005: // Mismatch nick or password, re-enter
	{
		
		reason = i18n("Could not sign on to %1 with account %2 as the " \
			"password was incorrect.").arg( acctType ).arg( client()->userId() );
		
		return true;
		break;
	}

	case 0x0007: // non-existant ICQ#
	case 0x0008: // non-existant ICQ#
	{
		reason = i18n("Could not sign on to %1 with nonexistent account %2.")
			.arg( acctType ).arg( client()->userId() );
		break;
	}

	case 0x0009: // Expired account
	{
		reason = i18n("Sign on to %1 failed because your account %2 expired.")
			.arg( acctType ).arg( client()->userId() );
		break;
	}

	case 0x0011: // Suspended account
	{
		reason = i18n("Sign on to %1 failed because your account %2 is " \
			"currently suspended.").arg( acctType ).arg( client()->userId() );
		break;
	}

	case 0x0015: // too many clients from same IP
	case 0x0016: // too many clients from same IP
	case 0x0017: // too many clients from same IP (reservation)
	{
		reason = i18n("Could not sign on to %1 as there are too many clients" \
			" from the same computer.").arg( acctType );
		break;
	}

	case 0x0018: // rate exceeded (turboing)
	{
		if ( client()->isLoggedIn() )
		{
			reason = i18n("Account %1 was blocked on the %2 server for" \
				" sending messages too quickly." \
				" Wait ten minutes and try again." \
				" If you continue to try, you will" \
				" need to wait even longer.")
				.arg( client()->userId() ).arg( acctType );
		}
		else
		{
			reason = i18n("Account %1 was blocked on the %2 server for" \
				" reconnecting too quickly." \
				" Wait ten minutes and try again." \
				" If you continue to try, you will" \
				" need to wait even longer.")
				.arg( client()->userId() ).arg( acctType) ;
		}
		break;
	}

	case 0x001C:
	{
		reason = i18n("The %1 server thinks the client you are using is " \
			"too old. Please report this as a bug at http://bugs.kde.org")
			.arg( acctType );
		break;
	}

	case 0x0022: // Account suspended because of your age (age < 13)
	{
		reason = i18n("Account %1 was disabled on the %2 server because " \
			"of your age (less than 13).")
		.arg( client()->userId() ).arg( acctType );
		break;
	}

	default:
	{
		if ( !client()->isLoggedIn() )
		{
			reason = i18n("Sign on to %1 with your account %2 failed.")
				.arg( acctType ).arg( client()->userId() );
		}
		break;
	}
	}

	return ( error > 0 );
}

#include "closeconnectiontask.moc"

//kate: tab-width 4; indent-mode csands;
