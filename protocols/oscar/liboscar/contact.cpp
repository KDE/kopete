/*
    Kopete Oscar Protocol
    OContact Object Implementation

    Copyright (c) 2006 Matt Rogers <mattr@kde.org>

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

#include "contact.h"
#include <QList>
#include <kdebug.h>
#include "oscarutils.h"
#include "buffer.h"

OContact::OContact()
{
	m_gid = 0;
	m_bid = 0;
	m_type = 0xFFFF;
	m_tlvLength = 0;
	m_waitingAuth = false;
	m_caps = 0;
}

OContact::OContact( const QString &name, int gid, int bid, int type, const QList<TLV> &tlvlist, int tlvLength )
{
	m_name = name;
	m_gid = gid;
	m_bid = bid;
	m_type = type;
	m_tlvLength = tlvLength;

	//deepcopy the tlvs
	m_tlvList = tlvlist;
	if ( m_tlvLength == 0 && !m_tlvList.isEmpty() )
		refreshTLVLength();

	checkTLVs();
}

OContact::OContact( const OContact& other )
{
	m_name = other.m_name;
	m_gid = other.m_gid;
	m_bid = other.m_bid;
	m_type = other.m_type;
	m_tlvLength = other.m_tlvLength;
	m_alias = other.m_alias;
	m_waitingAuth = other.m_waitingAuth;
	m_caps = other.m_caps;
	m_hash = other.m_hash;
	m_metaInfoId = other.m_metaInfoId;

	//deepcopy the tlvs
	m_tlvList = other.m_tlvList;
	if ( m_tlvLength == 0 && !m_tlvList.isEmpty() )
		refreshTLVLength();
}

bool OContact::isValid() const
{
	return m_type != 0xFFFF;
}

QString OContact::name() const
{
	return m_name;
}

quint16 OContact::gid() const
{
	return m_gid;
}

quint16 OContact::bid() const
{
	return m_bid;
}

bool OContact::supportsFeature( Oscar::Capability c ) const
{
	return ( m_caps & c );
}

quint16 OContact::type() const
{
	return m_type;
}

const QList<TLV>& OContact::tlvList() const
{
	return m_tlvList;
}

void OContact::setTLVListLength( quint16 newLength )
{
	m_tlvLength = newLength;
}

quint16 OContact::tlvListLength() const
{
	return m_tlvLength;
}

void OContact::setTLVList( QList<TLV> list )
{
	//deepcopy the tlvs
	m_tlvList = list;
	refreshTLVLength();
	checkTLVs();
}

void OContact::refreshTLVLength()
{
	m_tlvLength = 0;
	QList<TLV>::iterator it = m_tlvList.begin();
	for( ; it != m_tlvList.end(); ++it )
	{
		m_tlvLength += 4;
		m_tlvLength += (*it).length;
	}
}

void OContact::checkTLVs()
{
	//check for the auth TLV
	TLV authTLV = findTLV( m_tlvList, 0x0066 );
	if ( authTLV )
	{
		kDebug(14151) << "Need auth for contact " << m_name;
		m_waitingAuth = true;
	}
	else
		m_waitingAuth = false;

	//check for the alias TLV
	TLV aliasTLV = findTLV( m_tlvList, 0x0131 );
	if ( aliasTLV )
	{
		m_alias = QString::fromUtf8( aliasTLV.data, aliasTLV.length );
		kDebug( 14151 ) << "Got an alias '" << m_alias << "' for contact '" << m_name << "'";
	}
	else
		m_alias.clear();

	TLV privacyTLV = findTLV( m_tlvList, 0x00CA );
	if ( privacyTLV )
		kDebug(14151) << "Found privacy settings " << privacyTLV.data;

	TLV infoTLV = findTLV( m_tlvList, 0x00CC );
	if ( infoTLV )
		kDebug(14151) << "Found 'allow others to see...' options " << infoTLV.data;

	TLV metaInfoIdTLV = findTLV( m_tlvList, 0x015C );
	if ( metaInfoIdTLV )
	{
		m_metaInfoId = metaInfoIdTLV.data;
		kDebug( 14151 ) << "Got an meta info id '" << m_metaInfoId.toHex() << "' for contact '" << m_name << "'";
	}
	else
		m_metaInfoId.clear();
}

QString OContact::alias() const
{
	return m_alias;
}

void OContact::setAlias( const QString& newAlias )
{
	m_alias = newAlias;
}

bool OContact::waitingAuth() const
{
	return m_waitingAuth;
}

void OContact::setWaitingAuth( bool waiting )
{
	m_waitingAuth = waiting;
}

void OContact::setIconHash( QByteArray hash )
{
	m_hash = hash;
}

QByteArray OContact::iconHash( ) const
{
	return m_hash;
}

void OContact::setMetaInfoId( const QByteArray& id )
{
	m_metaInfoId = id;
}

QByteArray OContact::metaInfoId() const
{
	return m_metaInfoId;
}

QString OContact::toString() const
{
	QString ssiString = QString::fromLatin1( "name: " );
	ssiString += m_name;
	ssiString += " gid: ";
	ssiString += QString::number( m_gid );
	ssiString += " bid: ";
	ssiString += QString::number( m_bid );
	ssiString += " type: ";
	ssiString += QString::number( m_type );
	ssiString += " tlv length: ";
	ssiString += QString::number( m_tlvLength );
	return ssiString;
}

bool OContact::operator==( const OContact& item ) const
{
	if ( m_name == item.name() && m_gid == item.gid() && m_bid == item.bid() && m_type == item.type() )
		return true;
	else
		return false;
}

OContact::operator bool() const
{
	return isValid();
}

OContact::operator QByteArray() const
{
	Buffer b;
	QByteArray name( m_name.toUtf8() );
	b.addWord( name.length() );
	b.addString( name );
	b.addWord( m_gid );
	b.addWord( m_bid );
	b.addWord( m_type );
	b.addWord( m_tlvLength );
	QList<Oscar::TLV>::const_iterator it = m_tlvList.begin();
	for( ; it != m_tlvList.end(); ++it )
	{
		b.addWord( (*it).type );
		b.addWord( (*it).length );
		b.addString( (*it).data );
	}

	return (QByteArray) b;
}
