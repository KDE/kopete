/*
    Kopete Oscar Protocol
    oscarguid.cpp - Oscar Guid Object

    Kopete (c) 2002-2005 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "oscarguid.h"
#include <QString>

Oscar::Guid::Guid()
{
}

Oscar::Guid::Guid( const QByteArray& data )
: m_data( data )
{
}

Oscar::Guid::Guid( const QString& data )
{
	//get rid of the const
	QString d( data );
	//strip out dashes
	d.remove( '-' );
	//get each of the 16 2-char bytes
	bool ok = 1;
	for ( int i=0; i<32 && ok; i+=2 )
		m_data.append( d.mid( i, 2 ).toShort( &ok, 16 ) );
}

Oscar::Guid::Guid( const Guid& other )
: m_data( other.m_data )
{
}

const QByteArray Oscar::Guid::data() const
{
	return m_data;
}

void Oscar::Guid::setData( const QByteArray& data )
{
	m_data = data;
}

bool Oscar::Guid::isValid() const
{
	return m_data.size() == 16;
}

bool Oscar::Guid::isEqual( const Guid &rhs, int n ) const
{
	if( n > 16 )
		n = 16;
	return m_data.left(n) == rhs.m_data.left(n);
}

Oscar::Guid &Oscar::Guid::operator=( const Oscar::Guid& rhs )
{
	m_data = QByteArray( rhs.m_data.data(), 16 );
    //m_data = rhs.m_data; // has problem, Qt bug?!
	return *this;
}

bool Oscar::Guid::operator==( const Oscar::Guid& rhs ) const
{
	return m_data == rhs.m_data;
}

Oscar::Guid::operator QByteArray() const
{
	return m_data;
}

//kate: indent-mode csands; auto-insert-doxygen on; tab-width 4;
