/*
    Kopete Oscar Protocol
    oscartypeclasses.cpp - Oscar Type Definitions

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

#include "oscartypeclasses.h"
#include <qdeepcopy.h>
#include <qvaluelist.h>
#include <kdebug.h>
#include "oscarutils.h"


using namespace Oscar;

TLV::TLV()
{
	type = 0;
	length = 0;
}

TLV::TLV( Q_UINT16 newType, Q_UINT16 newLength, char* newData )
{
	type = newType;
	length = newLength;
	data.truncate(0);
	data.duplicate( newData, length );
}

TLV::TLV( Q_UINT16 newType, Q_UINT16 newLength, const QByteArray& newData )
{
	type = newType;
	length = newLength;
	data.duplicate( newData );
}

TLV::TLV( const TLV& t )
{
	type = t.type;
	length = t.length;
	data.truncate(0);
	data.duplicate( t.data );
}

TLV::~TLV()
{
}

TLV::operator bool() const
{
	return type != 0;
}


Oscar::Message::Message()
{
	m_channel = -1;
	m_properties = -1;
}

Oscar::Message::Message( const QString& text, int channel, int properties, QDateTime timestamp )
{
	m_text = text;
	m_channel = channel;
	m_properties = properties;
	m_timestamp = timestamp;
}

Oscar::Message::Message( const Oscar::Message& m )
{
	m_text = m.m_text;
	m_channel = m.m_channel;
	m_properties = m.m_properties;
	m_timestamp = m.m_timestamp;
}

QString Oscar::Message::sender() const
{
	return m_sender;
}

void Oscar::Message::setSender( const QString& sender  )
{
	m_sender = sender;
}

QString Oscar::Message::receiver() const
{
	return m_receiver;
}

void Oscar::Message::setReceiver( const QString& receiver )
{
	m_receiver = receiver;
}


QString Oscar::Message::text() const
{
	return m_text;
}

void Oscar::Message::setText( const QString& newText )
{
	m_text = newText;
}

int Oscar::Message::properties() const
{
	return m_properties;
}

void Oscar::Message::addProperty( int prop )
{
	if ( m_properties == -1  )
		m_properties = 0;

	m_properties = m_properties | prop;
}

int Oscar::Message::type() const
{
	return m_channel;
}

void Oscar::Message::setType( int newType )
{
	m_channel = newType;
}

QDateTime Oscar::Message::timestamp() const
{
	return m_timestamp;
}

void Oscar::Message::setTimestamp( QDateTime ts )
{
	m_timestamp = ts;
}

Oscar::Message::operator bool() const
{
	return m_channel != -1 && m_properties != -1;
}

SSI::SSI()
{
	m_gid = 0;
	m_bid = 0;
	m_type = 0xFFFF;
	m_tlvLength = 0;
	m_waitingAuth = false;
}

SSI::SSI( const QString &name, int gid, int bid, int type, const QValueList<TLV> &tlvlist, int tlvLength )
{
	m_name = name;
	m_gid = gid;
	m_bid = bid;
	m_type = type;
	m_tlvLength = tlvLength;
	
	//deepcopy the tlvs
	m_tlvList = QDeepCopy< QValueList<TLV> >( tlvlist );
	
	if ( m_tlvLength == 0 && !m_tlvList.isEmpty() )
		refreshTLVLength();
	
	checkTLVs();
}

SSI::SSI( const SSI& other )
{
	m_name = other.m_name;
	m_gid = other.m_gid;
	m_bid = other.m_bid;
	m_type = other.m_type;
	m_tlvLength = other.m_tlvLength;
	m_alias = other.m_alias;
	m_waitingAuth = other.m_waitingAuth;
		
	//deepcopy the tlvs
	m_tlvList = QDeepCopy< QValueList<TLV> >( other.m_tlvList );
	
	if ( m_tlvLength == 0 && !m_tlvList.isEmpty() )
		refreshTLVLength();
}

SSI::~SSI()
{
}

bool SSI::isValid() const
{
	return m_type != 0xFFFF;
}

QString SSI::name() const
{
	return m_name;
}

Q_UINT16 SSI::gid() const
{
	return m_gid;
}

Q_UINT16 SSI::bid() const
{
	return m_bid;
}

Q_UINT16 SSI::type() const
{
	return m_type;
}

const QValueList<TLV>& SSI::tlvList() const
{
	return m_tlvList;
}

void SSI::setTLVListLength( Q_UINT16 newLength )
{
	m_tlvLength = newLength;
}

Q_UINT16 SSI::tlvListLength() const
{
	return m_tlvLength;
}

void SSI::setTLVList( QValueList<TLV> list )
{
	//deepcopy the tlvs
	m_tlvList = QDeepCopy< QValueList<TLV> >( list );
	refreshTLVLength();
	checkTLVs();
}

void SSI::refreshTLVLength()
{
	QValueList<TLV>::iterator it = m_tlvList.begin();
	for( ; it != m_tlvList.end(); ++it )
	{
		m_tlvLength += 4;
		m_tlvLength += (*it).length;
	}
}

void SSI::checkTLVs()
{
	Buffer evil;
	QValueList<TLV>::iterator it = m_tlvList.begin();
	for( ; it != m_tlvList.end(); ++it )
		evil.addWord( ( *it ).type );
	
	kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "item has the following TLVs: "
		<< evil.getBlock( evil.length() ) << endl;
	
	//check for the auth TLV
	TLV authTLV = findTLV( m_tlvList, 0x0066 );
	if ( authTLV )
	{
		kdDebug(14151) << k_funcinfo << "Need auth for contact " << m_name << endl;
		m_waitingAuth = true;
	}
	else
		m_waitingAuth = false;
	
	//check for the alias TLV
	TLV aliasTLV = findTLV( m_tlvList, 0x0131 );
	if ( aliasTLV )
	{
		m_alias.insert( 0, aliasTLV.data );
		kdDebug( 14151 ) << k_funcinfo << "Got an alias '" << m_alias << "' for contact '" << m_name << "'" << endl;
	}
	
	TLV privacyTLV = findTLV( m_tlvList, 0x00CA );
	if ( privacyTLV )
		kdDebug(14151) << k_funcinfo << "Found privacy settings " << privacyTLV.data << endl;

	TLV infoTLV = findTLV( m_tlvList, 0x00CC );
	if ( infoTLV )
		kdDebug(14151) << k_funcinfo << "Found 'allow others to see...' options " << infoTLV.data << endl;
}

QString SSI::alias() const
{
	return m_alias;
}

void SSI::setAlias( const QString& newAlias )
{
	m_alias = newAlias;
}

bool SSI::waitingAuth() const
{
	return m_waitingAuth;
}

void SSI::setWaitingAuth( bool waiting )
{
	m_waitingAuth = waiting;
}

QString SSI::toString() const
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

bool SSI::operator==( const SSI& item ) const
{
	if ( m_name == item.name() && m_gid == item.gid() && m_bid == item.bid() && m_type == item.type() )
		return true;
	else
		return false;
}

SSI::operator bool() const
{
	return isValid();
}

//kate: indent-mode csands;
