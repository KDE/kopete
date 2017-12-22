/*
    Kopete Oscar Protocol
    oscarutils.cpp - Oscar Utility Functions

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

#include "oscarutils.h"

#include <QHostAddress>
#include <QTextCodec>

#include <kdebug.h>


using namespace Oscar;

QString Oscar::normalize( const QString& contact )
{
	QString normal = contact.toLower();
	normal.remove( QChar( ' ' ) );
	return normal;
}

bool Oscar::operator==( TLV a, TLV b )
{
	if ( a.type == b.type && a.length == b.length )
		return true;
	else
		return false;
}

TLV Oscar::findTLV( const QList<TLV>& list, int type )
{
	TLV t;
	QList<TLV>::const_iterator it;
	for ( it = list.begin(); it != list.end(); ++it )
	{
		if ( ( *it ).type == type )
			return ( *it );
	}

	return t;
}

bool Oscar::updateTLVs( OContact& item, const QList<TLV>& list )
{
	bool changed = false;
	QList<TLV> tList( item.tlvList() );

	QList<TLV>::const_iterator it;
	for ( it = list.begin(); it != list.end(); ++it )
	{
		TLV t = Oscar::findTLV( tList, ( *it ).type );
		if ( t && t.length == ( *it ).length &&
		     memcmp( t.data.data(), ( *it ).data.data(), t.length ) == 0 )
			continue;

		if ( t )
			tList.removeAll( t );

		tList.append( *it );
		changed = true;
	}

	if ( changed )
		item.setTLVList( tList );

	return changed;
}

const QString Oscar::capName( int capNumber )
{
	QString capString;

	switch ( capNumber )
	{
	case CAP_VOICE:
		capString = QStringLiteral("CAP_VOICE ");
		break;
	case CAP_BUDDYICON:
		capString = QStringLiteral("CAP_BUDDYICON ");
		break;
	case CAP_IMIMAGE:
		capString = QStringLiteral("CAP_IMIMAGE ");
		break;
	case CAP_CHAT:
		capString = QStringLiteral("CAP_CHAT ");
		break;
	case CAP_GETFILE:
		capString = QStringLiteral("CAP_GETFILE ");
		break;
	case CAP_SENDFILE:
		capString = QStringLiteral("CAP_SENDFILE ");
		break;
	case CAP_GAMES2:
	case CAP_GAMES:
		capString = QStringLiteral("CAP_GAMES ");
		break;
	case CAP_SAVESTOCKS:
		capString = QStringLiteral("CAP_SAVESTOCKS ");
		break;
	case CAP_SENDBUDDYLIST:
		capString = QStringLiteral("CAP_SENDBUDDYLIST ");
		break;
	case CAP_DIRECT_ICQ_COMMUNICATION:
		capString = QStringLiteral("CAP_DIRECT_ICQ_COMMUNICATION ");
		break;
	case CAP_APINFO:
		capString = QStringLiteral("CAP_APINFO ");
		break;
	case CAP_RTFMSGS:
		capString = QStringLiteral("CAP_RTFMSGS ");
		break;
	case CAP_ICQSERVERRELAY:
		capString = QStringLiteral("CAP_ICQSERVERRELAY ");
		break;
	case CAP_IS_2001:
		capString = QStringLiteral("CAP_IS_2001 ");
		break;
	case CAP_TRILLIAN:
		capString = QStringLiteral("CAP_TRILLIAN ");
		break;
	case CAP_TRILLIANCRYPT:
		capString = QStringLiteral("CAP_TRILLIANCRYPT ");
		break;
	case CAP_UTF8:
		capString = QStringLiteral("CAP_UTF8 ");
		break;
	case CAP_TYPING:
		capString = QStringLiteral("CAP_TYPING ");
		break;
	case CAP_INTEROPERATE:
		capString = QStringLiteral("CAP_INTEROPERATE ");
		break;
	case CAP_KOPETE:
		capString = QStringLiteral("CAP_KOPETE ");
		break;
	case CAP_MIRANDA:
		capString = QStringLiteral("CAP_MIRANDA");
		break;
	case CAP_QIP:
		capString = QStringLiteral("CAP_QIP");
		break;
	case CAP_QIPINFIUM:
		capString = QStringLiteral("CAP_QIPINFIUM");
		break;
	case CAP_QIPPDA:
		capString = QStringLiteral("CAP_QIPPDA");
		break;
	case CAP_QIPSYMBIAN:
		capString = QStringLiteral("CAP_QIPSYMBIAN");
		break;
	case CAP_QIPMOBILE:
		capString = QStringLiteral("CAP_QIPMOBILE");
		break;
	case CAP_JIMM:
		capString = QStringLiteral("CAP_JIMM");
		break;
	case CAP_VMICQ:
		capString = QStringLiteral("CAP_VMICQ");
		break;
	case CAP_LICQ:
		capString = QStringLiteral("CAP_LICQ");
		break;
	case CAP_ANDRQ:
		capString = QStringLiteral("CAP_ANDRQ");
		break;
	case CAP_RANDQ:
		capString = QStringLiteral("CAP_RANDQ");
		break;
	case CAP_ICQ_RAMBLER:
		capString = QStringLiteral("CAP_ICQ_RAMBLER");
		break;
	case CAP_ICQ_ABV:
		capString = QStringLiteral("CAP_ICQ_ABV");
		break;
	case CAP_ICQ_NETVIGATOR:
		capString = QStringLiteral("CAP_ICQ_NETVIGATOR");
		break;
	case CAP_TZERS:
		capString = QStringLiteral("CAP_TZERS");
		break;
	case CAP_HTMLMSGS:
		capString = QStringLiteral("CAP_HTMLMSGS");
		break;
	case CAP_MICQ:
		capString = QStringLiteral("CAP_MICQ ");
		break;
	case CAP_MACICQ:
		capString = QStringLiteral("CAP_MACICQ ");
		break;
	case CAP_SIMOLD:
		capString = QStringLiteral("CAP_SIMOLD ");
		break;
	case CAP_SIMNEW:
		capString = QStringLiteral("CAP_SIMNEW ");
		break;
	case CAP_XTRAZ:
		capString = QStringLiteral("CAP_XTRAZ ");
		break;
	case CAP_STR_2001:
		capString = QStringLiteral("CAP_STR_2001 ");
		break;
	case CAP_STR_2002:
		capString = QStringLiteral("CAP_STR_2002 ");
		break;
	case CAP_XTRAZ_MULTIUSER_CHAT:
		capString = QStringLiteral("CAP_XTRAZ_MULTIUSER_CHAT ");
		break;
	case CAP_DEVILS:
		capString = QStringLiteral("CAP_DEVILS ");
		break;
	case CAP_NEWCAPS:
		capString = QStringLiteral("CAP_NEWCAPS ");
		break;
	case CAP_UNKNOWN2:
		capString = QStringLiteral("CAP_UNKNOWN2 ");
		break;
	case CAP_PUSH2TALK:
		capString = QStringLiteral("CAP_PUSH2TALK ");
		break;
	case CAP_VIDEO:
		capString = QStringLiteral("CAP_VIDEO ");
		break;
	default:
		capString = QStringLiteral("UNKNOWN CAP ");
	} // END switch

	return capString;
}

Oscar::DWORD Oscar::getNumericalIP(const QString &address)
{
	QHostAddress addr;
	if ( addr.setAddress( address ) == false )
		return 0;

	return (DWORD)addr.toIPv4Address();
}

QString Oscar::getDottedDecimal( Oscar::DWORD address )
{
	QHostAddress addr;
	addr.setAddress((quint32)address);
	return addr.toString();
}

QTextCodec * Oscar::codecForName( const QByteArray& name )
{
	if ( name == "iso-8859-1" || name == "us-ascii" )
		return QTextCodec::codecForName( "ISO 8859-1" );
	else if ( name == "utf-8" )
		return QTextCodec::codecForName( "UTF-8" );
	else
		return QTextCodec::codecForName( name );
}


//kate: tab-width 4; indent-mode csands;
