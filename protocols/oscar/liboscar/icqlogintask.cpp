/*
    Kopete Oscar Protocol
    icqlogintask.cpp - Handles logging into to the ICQ service

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

#include "icqlogintask.h"

#include <qstring.h>
#include <kdebug.h>
#include "transfer.h"
#include "connection.h"
#include "oscarutils.h"

using namespace Oscar;

IcqLoginTask::IcqLoginTask( Task* parent )
	: Task( parent )
{
}

IcqLoginTask::~IcqLoginTask()
{
}

bool IcqLoginTask::take( Transfer* transfer )
{
	Q_UNUSED( transfer );
	return false;
}

bool IcqLoginTask::forMe( Transfer* transfer ) const
{
	//there shouldn't be a incoming transfer for this task
	Q_UNUSED( transfer );
	return false;
}

void IcqLoginTask::onGo()
{
	FLAP f = { 0x01, 0, 0 };
	DWORD flapVersion = 0x00000001;
	Buffer *outbuf = new Buffer();

	QString encodedPassword = encodePassword( client()->password() );
	const Oscar::ClientVersion* version = client()->version();
	
	outbuf->addDWord( flapVersion );
	outbuf->addTLV( 0x0001, client()->userId().length(), client()->userId().latin1() );
	outbuf->addTLV( 0x0002, encodedPassword.length(), encodedPassword.latin1() );
	outbuf->addTLV( 0x0003, version->clientString.length(), version->clientString.latin1() );
	outbuf->addTLV16( 0x0016, version->clientId );
	outbuf->addTLV16( 0x0017, version->major );
	outbuf->addTLV16( 0x0018, version->minor );
	outbuf->addTLV16(  0x0019, version->point );
	outbuf->addTLV16(0x001a, version->build );
	outbuf->addDWord( 0x00140004 ); //TLV type 0x0014, length 0x0004
	outbuf->addDWord( version->other ); //TLV data for type 0x0014
	outbuf->addTLV( 0x000f, version->lang.length(), version->lang.latin1() );
	outbuf->addTLV( 0x000e, version->country.length(), version->country.latin1() );

	Transfer* ft = createTransfer( f, outbuf );
	kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Sending ICQ channel 0x01 login packet" << endl;
	send( ft );
	emit finished();
}


QString IcqLoginTask::encodePassword( const QString& loginPassword )
{
	kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Called." << endl;

	// TODO: check if latin1 is the right conversion
	const char *password = loginPassword.latin1();
	unsigned int i = 0;
	QString encodedPassword = QString::null;

	//encoding table used in ICQ password XOR transformation
	unsigned char encoding_table[] =
	{
		0xf3, 0x26, 0x81, 0xc4,
		0x39, 0x86, 0xdb, 0x92,
		0x71, 0xa3, 0xb9, 0xe6,
		0x53, 0x7a, 0x95, 0x7c
	};
	
	for (i = 0; i < 8; i++)
	{
		if(password[i] == 0)
			break; //found a null in the password. don't encode it
		encodedPassword.append( password[i] ^ encoding_table[i] );
	}

#ifdef OSCAR_PWDEBUG
	kdDebug(OSCAR_RAW_DEBUG) << " plaintext pw='" << loginPassword << "', length=" <<
		loginPassword.length() << endl;

	kdDebug(OSCAR_RAW_DEBUG) << " encoded   pw='" << encodedPassword  << "', length=" <<
		encodedPassword.length() << endl;
#endif

	return encodedPassword;
}

#include "icqlogintask.moc"
