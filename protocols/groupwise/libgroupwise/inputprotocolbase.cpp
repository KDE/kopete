//
// C++ Implementation: %{MODULE}
//
// Description: 
//
//
// Author: %{AUTHOR} <%{EMAIL}>, (C) %{YEAR}
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "inputprotocolbase.h"

#include "gwfield.h"

InputProtocolBase::InputProtocolBase(QObject *parent, const char *name)
 : QObject(parent, name)
{
}


InputProtocolBase::~InputProtocolBase()
{
}

uint InputProtocolBase::state() const
{
	return m_state;
}

bool InputProtocolBase::readString( QString &message )
{
	uint len;
	QCString rawData;
	if ( !safeReadBytes( rawData, len ) )
		return false;
	message = QString::fromUtf8( rawData.data(), len );
	return true;
}


bool InputProtocolBase::okToProceed()
{
	if ( m_din )
	{
		if ( m_din->atEnd() )
		{
			m_state = NeedMore;
			qDebug( "EventProtocol::okToProceed() - Server message ended prematurely!" );
		}
		else
			return true;
	}
	return false;
}

bool InputProtocolBase::safeReadBytes( QCString & data, uint & len )
{
	// read the length of the bytes
	Q_UINT32 val;
	if ( !okToProceed() )
		return false;
	*m_din >> val;
	m_bytes += sizeof( Q_UINT32 );
	if ( val > NMFIELD_MAX_STR_LENGTH )
		return false;
	qDebug( "EventProtocol::safeReadBytes() - expecting %i bytes", val );
	QCString temp( val );
	if ( val != 0 )
	{
		if ( !okToProceed() )
			return false;
		// if the server splits packets here we are in trouble,
		// as there is no way to see how much data was actually read
		m_din->readRawBytes( temp.data(), val );
		// the rest of the string will be filled with FF,
		// so look for that in the last position instead of \0
		// this caused a crash - guessing that temp.length() is set to the number of bytes actually read...
		// if ( (Q_UINT8)( * ( temp.data() + ( temp.length() - 1 ) ) ) == 0xFF )
		if ( temp.length() < ( val -1 ) )
		{
			qDebug( "EventProtocol::safeReadBytes() - string broke, giving up, only got: %i bytes",  temp.length() );
			m_state = NeedMore;
			return false;
		}
	}
	data = temp;
	len = val;
	m_bytes += val;
	return true;
}

#include "inputprotocolbase.moc"
