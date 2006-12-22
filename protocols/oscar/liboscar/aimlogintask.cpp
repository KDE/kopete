/*
    Kopete Oscar Protocol
    aimlogintask.h - Handles logging into to the AIM service

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
#include "aimlogintask.h"

#include <stdlib.h>
#include <kdebug.h>
#include <klocale.h>
#include "connection.h"
#include "oscartypes.h"
#include "oscarutils.h"
#include "transfer.h"

#include "md5.h"

using namespace Oscar;

AimLoginTask::AimLoginTask( Task* parent )
	: Task ( parent )
{
}

AimLoginTask::~AimLoginTask()
{
}

void AimLoginTask::onGo()
{
	//send Snac 17,06
	sendAuthStringRequest();
	//when we have the authKey, login
	connect( this, SIGNAL( haveAuthKey() ), this, SLOT( sendLoginRequest() ) );
}

bool AimLoginTask::forMe( Transfer* transfer ) const
{
	SnacTransfer* st = dynamic_cast<SnacTransfer*>( transfer );

	if (!st)
		return false;

	if ( st && st->snacService() == 0x17 )
	{
		WORD subtype = st->snacSubtype();
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

const QByteArray& AimLoginTask::cookie() const
{
	return m_cookie;
}

const QString& AimLoginTask::bosHost() const
{
	return m_bosHost;
}

const QString& AimLoginTask::bosPort() const
{
	return m_bosPort;
}

bool AimLoginTask::take( Transfer* transfer )
{
	if ( forMe( transfer ) )
	{
		SnacTransfer* st = dynamic_cast<SnacTransfer*>( transfer );
		if (!st)
			return false;

		WORD subtype = st->snacSubtype();
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

void AimLoginTask::sendAuthStringRequest()
{
	kDebug(OSCAR_RAW_DEBUG) << k_funcinfo
		<< "SEND CLI_AUTH_REQUEST, sending login request" << endl;

	FLAP f = { 0x02, 0, 0 };
	SNAC s = { 0x0017, 0x0006, 0x0000, client()->snacSequence() };

	Buffer* outbuf = new Buffer;
	outbuf->addTLV(0x0001, client()->userId().toLatin1() );
	outbuf->addDWord(0x004B0000); // empty TLV 0x004B
	outbuf->addDWord(0x005A0000); // empty TLV 0x005A

	Transfer* st = createTransfer( f, s, outbuf );
	send( st );
}

QByteArray AimLoginTask::parseAuthString( Buffer* b )
{
	WORD keylength = b->getWord();
	kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Key length is " << keylength << endl;
	QByteArray authString = b->getBlock( keylength );
	return authString;
}


void AimLoginTask::processAuthStringReply()
{
	kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Got the authorization key" << endl;
	m_authKey = parseAuthString( transfer()->buffer() );
	emit haveAuthKey();
}

void AimLoginTask::sendLoginRequest()
{
	kDebug(OSCAR_RAW_DEBUG) << k_funcinfo <<  "SEND (CLI_MD5_LOGIN) sending AIM login" << endl;

	FLAP f = { 0x02, 0, 0 };
	SNAC s = { 0x0017, 0x0002, 0x0000, client()->snacSequence() };
	Buffer *outbuf = new Buffer;
	outbuf->addTLV(0x0001, client()->userId().toLatin1());

	QByteArray digest;
	digest.resize( 16 ); //apparently MD5 digests are 16 bytes long
	encodePassword( digest );

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

	//if set, old-style buddy lists will not work... you will need to use SSI
	outbuf->addTLV8(0x004a,0x01);

	Transfer *st = createTransfer( f, s, outbuf );
	send( st );
}

void AimLoginTask::handleLoginResponse()
{
	kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "RECV SNAC 0x17, 0x07 - AIM Login Response" << endl;

	SnacTransfer* st = dynamic_cast<SnacTransfer*> ( transfer() );

	if ( !st )
	{
		setError( -1 , QString::null );
		return;
	}

	QList<TLV> tlvList = st->buffer()->getTLVList();

	TLV uin = findTLV( tlvList, 0x0001 );
	if ( uin )
	{
		kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "found TLV(1) [SN], SN=" << QString( uin.data ) << endl;
	}

	TLV err = findTLV( tlvList, 0x0008 );

	if ( err )
	{
		WORD errorNum = ( ( err.data[0] << 8 ) | err.data[1] );

		kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << k_funcinfo << "found TLV(8) [ERROR] error= " <<
			errorNum << endl;
		Oscar::SNAC s = { 0, 0, 0, 0 };
		client()->fatalTaskError( s, errorNum );
		setError( errorNum, QString::null );
		return; //if there's an error, we'll need to disconnect anyways
	}

	TLV server = findTLV( tlvList, 0x0005 );
	if ( server )
	{
		QString ip = QString( server.data );
		kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "found TLV(5) [SERVER] " << ip << endl;
		int index = ip.indexOf( ':' );
		m_bosHost = ip.left( index );
		ip.remove( 0 , index+1 ); //get rid of the colon and everything before it
		m_bosPort = ip.left(4); //we only need 4 bytes
		kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "We should reconnect to server '" << m_bosHost <<
			"' on port " << m_bosPort << endl;
	}

	TLV cookie = findTLV( tlvList, 0x0006 );
	if ( cookie )
	{
		kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "found TLV(6) [COOKIE]" << endl;
		m_cookie = cookie.data;
		setSuccess( 0, QString::null );
	}
	tlvList.clear();
}

void AimLoginTask::encodePassword( QByteArray& digest ) const
{
	kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << endl;
	md5_state_t state;
	md5_init( &state );
	md5_append( &state, ( const md5_byte_t* ) m_authKey.data(), m_authKey.size() );
	md5_append( &state, ( const md5_byte_t* ) client()->password().toLatin1().data(), client()->password().length() );
	md5_append( &state, ( const md5_byte_t* ) AIM_MD5_STRING, strlen( AIM_MD5_STRING ) );
	md5_finish( &state, ( md5_byte_t* ) digest.data() );
}

//kate: indent-mode csands; tab-width 4; replace-tabs off; indent-spaces off;

#include "aimlogintask.moc"
