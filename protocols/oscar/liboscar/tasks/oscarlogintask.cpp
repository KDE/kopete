/*
    Kopete Oscar Protocol
    oscarlogintask.h - Handles logging into to the OSCAR service

    Copyright (c) 2004 Matt Rogers <mattr@kde.org>
    Copyright (c) 2007 Roman Jarosz <kedgedev@centrum.cz>

    Kopete (c) 2002-2007 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/
#include "oscarlogintask.h"

#include <QtCore/QCryptographicHash>

#include <stdlib.h>
#include <kdebug.h>
#include <klocale.h>
#include "connection.h"
#include "oscartypes.h"
#include "oscarutils.h"
#include "transfer.h"

using namespace Oscar;

OscarLoginTask::OscarLoginTask( Task* parent )
	: Task ( parent )
{
}

OscarLoginTask::~OscarLoginTask()
{
}

void OscarLoginTask::onGo()
{
	//send Snac 17,06
	sendAuthStringRequest();
	//when we have the authKey, login
	connect( this, SIGNAL(haveAuthKey()), this, SLOT(sendLoginRequest()) );
}

bool OscarLoginTask::forMe( const Transfer* transfer ) const
{
	const SnacTransfer* st = dynamic_cast<const SnacTransfer*>( transfer );

	if (!st)
		return false;

	if ( st && st->snacService() == 0x17 )
	{
		Oscar::WORD subtype = st->snacSubtype();
		switch ( subtype )
		{
		case 0x0002:
		case 0x0003:
		case 0x0006:
		case 0x0007:
			return true;
			break;
		default:
			return false;
			break;
		}
	}
	return false;
}

const QByteArray& OscarLoginTask::cookie() const
{
	return m_cookie;
}

const QString& OscarLoginTask::bosHost() const
{
	return m_bosHost;
}

const QString& OscarLoginTask::bosPort() const
{
	return m_bosPort;
}

bool OscarLoginTask::bosEncrypted() const
{
	return m_bosEncrypted;
}

const QString& OscarLoginTask::bosSSLName() const
{
	return m_bosSSLName;
}

bool OscarLoginTask::take( Transfer* transfer )
{
	if ( forMe( transfer ) )
	{
		SnacTransfer* st = dynamic_cast<SnacTransfer*>( transfer );
		if (!st)
			return false;

		Oscar::WORD subtype = st->snacSubtype();
		switch ( subtype )
		{
		case 0x0003:
			setTransfer( transfer );
			handleLoginResponse();
			setTransfer( 0 );
			return true;
			break;
		case 0x0007:
			setTransfer( transfer );
			processAuthStringReply();
			setTransfer( 0 );
			return true;
			break;
		default:
			return false;
			break;
		}

		return false;
	}
	return false;
}

void OscarLoginTask::sendAuthStringRequest()
{
	kDebug(OSCAR_RAW_DEBUG) 
		<< "SEND CLI_AUTH_REQUEST, sending login request" << endl;

	FLAP f = { 0x02, 0, 0 };
	SNAC s = { 0x0017, 0x0006, 0x0000, client()->snacSequence() };

	Buffer* outbuf = new Buffer;
	outbuf->addTLV(0x0001, client()->userId().toLatin1() );

	Transfer* st = createTransfer( f, s, outbuf );
	send( st );
}

void OscarLoginTask::processAuthStringReply()
{
	kDebug(OSCAR_RAW_DEBUG) << "Got the authorization key";

	Buffer* b = transfer()->buffer();
	m_authKey = b->getBSTR();

	emit haveAuthKey();
}

void OscarLoginTask::sendLoginRequest()
{
	kDebug(OSCAR_RAW_DEBUG) <<  "SEND (CLI_MD5_LOGIN) sending AIM login";

	FLAP f = { 0x02, 0, 0 };
	SNAC s = { 0x0017, 0x0002, 0x0000, client()->snacSequence() };
	Buffer *outbuf = new Buffer;
	outbuf->addTLV(0x0001, client()->userId().toLatin1());

	QByteArray digest = encodePassword();

	const Oscar::ClientVersion* version = client()->version();
	outbuf->addTLV(0x0025, digest );
	outbuf->addTLV(0x0003, version->clientString.toLatin1() );
	outbuf->addTLV16(0x0016, version->clientId );
	outbuf->addTLV16(0x0017, version->major );
	outbuf->addTLV16(0x0018, version->minor );
	outbuf->addTLV16(0x0019, version->point );
	outbuf->addTLV16(0x001a, version->build );
	outbuf->addTLV32(0x0014, version->other );
	outbuf->addTLV(0x000f, version->lang.toLatin1() );
	outbuf->addTLV(0x000e, version->country.toLatin1() );

	if ( !client()->isIcq() )
	{
		//if set, old-style buddy lists will not work... you will need to use SSI
		outbuf->addTLV8(0x004a,0x01);
	}

	Transfer *st = createTransfer( f, s, outbuf );
	send( st );
}

void OscarLoginTask::handleLoginResponse()
{
	kDebug(OSCAR_RAW_DEBUG) << "RECV SNAC 0x17, 0x07 - AIM Login Response";

	SnacTransfer* st = dynamic_cast<SnacTransfer*> ( transfer() );

	if ( !st )
	{
		setError( -1 , QString() );
		return;
	}

	QList<TLV> tlvList = st->buffer()->getTLVList();

	TLV uin = findTLV( tlvList, 0x0001 );
	if ( uin )
	{
		kDebug(OSCAR_RAW_DEBUG) << "found TLV(1) [SN], SN=" << QString( uin.data );
	}

	TLV err = findTLV( tlvList, 0x0008 );	
	if ( err )
	{
		Oscar::WORD errorNum = ( ( err.data[0] << 8 ) | err.data[1] );

		kDebug(OSCAR_RAW_DEBUG) << "found TLV(8) [ERROR] error= " << errorNum;
		Oscar::SNAC s = { 0, 0, 0, 0 };
		client()->fatalTaskError( s, errorNum );
		setError( errorNum, QString() );
		return; //if there's an error, we'll need to disconnect anyways
	}

	TLV server = findTLV( tlvList, 0x0005 );
	if ( server )
	{
		kDebug(OSCAR_RAW_DEBUG) << "found TLV(5) [SERVER] " << QString( server.data );
		QString ip = QString( server.data );
		int index = ip.indexOf( ':' );
		m_bosHost = ip.left( index );
		ip.remove( 0 , index+1 ); //get rid of the colon and everything before it
		m_bosPort = ip;
	}

	TLV cookie = findTLV( tlvList, 0x0006 );
	if ( cookie )
	{
		kDebug(OSCAR_RAW_DEBUG) << "found TLV(6) [COOKIE]";
		m_cookie = cookie.data;
	}

	TLV sslcert = findTLV( tlvList, 141 );
	if ( sslcert )
	{
		kDebug(OSCAR_RAW_DEBUG) << "found TLV(141) [SSLCERT]";
		m_bosSSLName = sslcert.data;
	}

	TLV ssl = findTLV( tlvList, 142 );
	{
		kDebug(OSCAR_RAW_DEBUG) << "found TLV(142) [SSL] " << (int)ssl.data[0];
		m_bosEncrypted = ssl.data[0];
	}

	tlvList.clear();

	if ( m_bosHost.isEmpty() )
	{
		kWarning(OSCAR_RAW_DEBUG) << "Empty host address!";
		
		Oscar::SNAC s = { 0, 0, 0, 0 };
		client()->fatalTaskError( s, 0 );
		setError( 0, QString() );
		return;
	}
	
	kDebug( OSCAR_RAW_DEBUG ) << "We should reconnect to server '"
		<< m_bosHost << "' on port " << m_bosPort << ( m_bosEncrypted ? " with " : " without " ) << "SSL" << endl;
	setSuccess( 0, QString() );
}

QByteArray OscarLoginTask::encodePassword() const
{
	kDebug(OSCAR_RAW_DEBUG) ;
	QCryptographicHash h(QCryptographicHash::Md5);
	h.addData( m_authKey );
	h.addData( client()->password().toLatin1() );
	h.addData( AIM_MD5_STRING, strlen( AIM_MD5_STRING ) );
	return h.result();
}

//kate: indent-mode csands; tab-width 4; replace-tabs off; indent-spaces off;

#include "oscarlogintask.moc"
