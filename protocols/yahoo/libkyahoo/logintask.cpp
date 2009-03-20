/*
    Kopete Yahoo Protocol
    Handles logging into to the Yahoo service

    Copyright (c) 2004 Duncan Mac-Vicar P. <duncan@kde.org>

    Copyright (c) 2005-2006 André Duffeck <duffeck@kde.org>

    Kopete (c) 2002-2006 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/



#include "logintask.h"
#include "transfer.h"
#include "ymsgtransfer.h"
#include "yahootypes.h"
#include "client.h"
#include <qstring.h>
#include <kdebug.h>
#include <stdlib.h>
extern "C"
{
#include "libyahoo.h"
}

LoginTask::LoginTask(Task* parent) : Task(parent)
{
	kDebug(YAHOO_RAW_DEBUG) ;
	mState = InitialState;
}

LoginTask::~LoginTask()
{

}

bool LoginTask::take(Transfer* transfer)
{
	/*
	  Yahoo login task has various stages
	  
	  1 .- Initial State
	      1.1 .- OnGo is called
	      1.2 .- SendVerify() - send a service verify ack
	  2.- SentVerify
	      2.1 - take(), get a useless transfer, sendAuth is called
	  3.- SentAuth
	      2.2 - take(), get a transfer with login and challenge string
	            sendAuthResp is called.
              2.3 - Need to decode and send a transfer back
	  4.- SentAuthResp
	*/
	
	if ( !forMe( transfer ) )
		return false;

	YMSGTransfer *t = static_cast<YMSGTransfer *>(transfer);


	if ( t->service() == Yahoo::ServicePing) {
		emit buddyListReady();
		return true;
	}

	switch (mState)
	{
		case (InitialState):
			client()->notifyError( "Error in login procedure.", "take called while in initial state", Client::Debug );
			return false;
		break;
		case (SentVerify):
			sendAuth( t );
			return true;
		break;
		case (SentAuth):
			sendAuthResp( t );
			return true;
		break;
		case (SentAuthResp):
			parseCookies( t );
			handleAuthResp( t );
			// Throw transfer to the next task as it contains further data
			return false;
		break;
		default:
		return false;
		break;
	}
}

bool LoginTask::forMe(const Transfer* transfer) const
{
	const YMSGTransfer *t = 0L;
	t = dynamic_cast<const YMSGTransfer*>(transfer);
	if (!t)
		return false;

	if ( t->service() == Yahoo::ServicePing)
		return true;

	switch (mState)
	{
		case (InitialState):
			//there shouldn't be a incoming transfer for this task at this state
			return false;
		break;
		case (SentVerify):
			if ( t->service() == Yahoo::ServiceVerify )
			return true;
		break;
		case (SentAuth):
			if ( t->service() == Yahoo::ServiceAuth )
			return true;
		break;
		case (SentAuthResp ):
			if ( t->service() == Yahoo::ServiceList ||
				t->service() == Yahoo::ServiceAuthResp )
			return true;
		default:
			return false;
		break;
	}
	return false;
}

void LoginTask::onGo()
{
	kDebug(YAHOO_RAW_DEBUG) ;
	/* initial state, we have to send a ServiceVerify */
	if (mState == InitialState)
		sendVerify();
	else
		client()->notifyError( "Error in login procedure.", "onGo called while not in initial state", Client::Debug );
}

void LoginTask::reset()
{
	mState = InitialState;
}

void LoginTask::sendVerify()
{
	/* send a ServiceVerify */
	kDebug(YAHOO_RAW_DEBUG) ;
	YMSGTransfer *t = new YMSGTransfer(Yahoo::ServiceVerify);
	send( t );
	mState = SentVerify;	
}

void LoginTask::sendAuth(YMSGTransfer* transfer)
{
	kDebug(YAHOO_RAW_DEBUG) ;
	// transfer is the verify ack transfer, no useful data in it.
	Q_UNUSED(transfer);
	
	/* got ServiceVerify ACK, send a ServiceAuth with username */
	kDebug(YAHOO_RAW_DEBUG) ;
	YMSGTransfer *t = new YMSGTransfer( Yahoo::ServiceAuth );
	t->setParam( 1 , client()->userId().toLocal8Bit() );
	send(t);
	mState = SentAuth;
}

void LoginTask::sendAuthResp(YMSGTransfer* t)
{
	kDebug(YAHOO_RAW_DEBUG) ;
	
	QString sn = t->firstParam( 1 );
	QString seed = t->firstParam( 94 );
	QString version_s = t->firstParam( 13 );
	uint sessionID = t->id();
	int version = version_s.toInt();
	
	switch (version)
	{
		case 0:
		kDebug(YAHOO_RAW_DEBUG) << " Version pre 0x0b "<< version_s;	
		break;
		default:
		kDebug(YAHOO_RAW_DEBUG) << " Version 0x0b "<< version_s;
		sendAuthResp_0x0b(sn, seed, sessionID);
		break;
	}	
	mState = SentAuthResp;

	emit haveSessionID( sessionID );
}

void LoginTask::sendAuthResp_0x0b(const QString &sn, const QString &seed, uint sessionID)
{
	kDebug(YAHOO_RAW_DEBUG) << " with seed " << seed;
	char *resp_6 = (char *) malloc(100);
	char *resp_96 = (char *) malloc(100);
	authresp_0x0b(seed.toLatin1(), sn.toLatin1(), (client()->password()).toLatin1(), resp_6, resp_96);
	kDebug(YAHOO_RAW_DEBUG) << "resp_6: " << resp_6 << " resp_69: " << resp_96;
	YMSGTransfer *t = new YMSGTransfer(Yahoo::ServiceAuthResp, m_stateOnConnect);
	t->setId( sessionID );
	t->setParam( 0 , sn.toLocal8Bit());
	t->setParam( 2 , sn.toLocal8Bit());
	t->setParam( 2, 1 ); // Both parameter 2s wind up in the packet
	t->setParam( 6 , resp_6);
	t->setParam( 1, sn.toLocal8Bit());
	t->setParam( 244, 2097087 );
	t->setParam( 135, YMSG_PROGRAM_VERSION_STRING );
	t->setParam( 148, 480 );
	t->setParam( 59 , "B\\tfckeert1kk1nl&b=2" );	// ???

	if( !m_verificationWord.isEmpty() )
	{
		t->setParam( 227 , m_verificationWord.toLocal8Bit() );
		m_verificationWord.clear();
	}

	free(resp_6);
	free(resp_96);
	send(t);

}

void LoginTask::sendAuthResp_pre_0x0b(const QString &/*sn*/, const QString &/*seed*/)
{
	kDebug(YAHOO_RAW_DEBUG) ;
}

void LoginTask::handleAuthResp(YMSGTransfer *t)
{
	kDebug(YAHOO_RAW_DEBUG) ;

	switch( t->service() )
	{
		case( Yahoo::ServiceList ):
			kDebug(YAHOO_RAW_DEBUG) << "Emitting Signal";
			emit loginResponse( Yahoo::LoginOk, QString() );
		break;
		case( Yahoo::ServiceAuthResp ):
			kDebug(YAHOO_RAW_DEBUG) << "Emitting Signal";
			emit loginResponse( t->firstParam( 66 ).toInt(), t->firstParam( 20 ) );
		break;
		default:
		break;
	}
	mState = InitialState;
}

void LoginTask::setStateOnConnect( Yahoo::Status status )
{
	m_stateOnConnect = status;
}

void LoginTask::parseCookies( YMSGTransfer *t )
{
	kDebug(YAHOO_RAW_DEBUG) ;

	for( int i = 0; i < t->paramCount( 59 ); ++i)
	{	
		QString cookie;
		cookie = t->nthParam( 59, i );
        	if( cookie.startsWith( 'Y' ) )
		{
			m_yCookie = getcookie( cookie.toLatin1() );
			m_loginCookie = getlcookie( cookie.toLatin1() );
		}
		else if( cookie.startsWith( 'T' ) )
		{
			m_tCookie = getcookie( cookie.toLatin1() );
		}
		else if( cookie.startsWith( 'C' ) )
		{
			m_cCookie = getcookie( cookie.toLatin1() );
		}
    	}
	if( !m_yCookie.isEmpty() && !m_tCookie.isEmpty() &&
		!m_cCookie.isEmpty() )
		emit haveCookies();
}

void LoginTask::setVerificationWord( const QString &word )
{
	m_verificationWord = word;
}

const QString& LoginTask::yCookie()
{
	return m_yCookie;
}

const QString& LoginTask::tCookie()
{
	return m_tCookie;
}

const QString& LoginTask::cCookie()
{
	return m_cCookie;
}

const QString& LoginTask::loginCookie()
{
	return m_loginCookie;
}
#include "logintask.moc"
