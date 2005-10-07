// Kopete Oscar Protocol - Server redirections

// Copyright (C)  2005  Matt Rogers <mattr@kde.org>

// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.

// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
// 02111-1307  USA

#include "serverredirecttask.h"

#include <kdebug.h>

#include "buffer.h"
#include "connection.h"
#include "transfer.h"

ServerRedirectTask::ServerRedirectTask( Task* parent )
	:Task( parent ),  m_service( 0 )
{

}

void ServerRedirectTask::setService( WORD family )
{
	m_service = family;
}

void ServerRedirectTask::setChatParams( WORD exchange, QByteArray cookie, WORD instance )
{
    m_chatExchange = exchange;
    m_chatCookie.duplicate( cookie );
    kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "cookie is" << m_chatCookie << endl;
    m_chatInstance = instance;
}

void ServerRedirectTask::setChatRoom( const QString& roomName )
{
    m_chatRoom = roomName;
}


void ServerRedirectTask::onGo()
{
	if ( m_service != 0 )
		requestNewService();
}

bool ServerRedirectTask::forMe( const Transfer* transfer )
{
	const SnacTransfer* st = dynamic_cast<const SnacTransfer*>( transfer );
	if ( !st )
		return false;

	if ( st->snacService() == 1 && st->snacSubtype() == 0x0005 )
		return true;
	else
		return false;
}

bool ServerRedirectTask::take( Transfer* transfer )
{
	if ( !forMe( transfer ) )
		return false;

	setTransfer( transfer );
    bool value = handleRedirect();
    setSuccess( 0, QString::null );
    setTransfer( 0 );
	return value;
}


void ServerRedirectTask::requestNewService()
{
	FLAP f = { 0x02, 0, 0x00 };
	SNAC s = { 0x0001, 0x0004, 0x0000, client()->snacSequence() };
	Buffer* b = new Buffer();
	b->addWord( m_service );
    kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Requesting server for service " << m_service << endl;
    if ( m_service == 0x000E )
    {
        b->addWord( 0x0001 );
        b->addWord( m_chatCookie.size() + 5 );
        b->addWord( m_chatExchange );
        b->addByte( m_chatCookie.size() );
        b->addString( m_chatCookie );
        b->addWord( m_chatInstance );
    }

    Transfer* t = createTransfer( f, s, b );
    send( t );
}

bool ServerRedirectTask::handleRedirect()
{
	//TLVs 0x0D, 0x05, 0x06
	//family id
	//server
	//auth cookie
	Buffer* b = transfer()->buffer();
	WORD typeD = b->getWord();
	WORD typeDLen = b->getWord();
	if ( typeD == 0x000D && typeDLen == 0x0002)
	{
        WORD realService = b->getWord();
		if ( realService != m_service )
		{
			kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "wrong service for this task" << endl;
			kdDebug(OSCAR_RAW_DEBUG ) << k_funcinfo << "should be " << m_service << " is "
			                          << realService << endl;
			return false;
		}
	}
	else
		return false;

	TLV server = b->getTLV();
	m_newHost = QString( server.data );
	kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Host for service " << m_service
	                         << " is " << m_newHost << endl;
	if ( m_newHost.isEmpty() )
		return false;

	TLV cookie = b->getTLV();

	if ( cookie.length == 0 || cookie.data.isEmpty() )
		return false;
	else
		m_cookie = cookie.data;

	emit haveServer( m_newHost, m_cookie, m_service );
	return true;
}

QByteArray ServerRedirectTask::cookie() const
{
	return m_cookie;
}

QString ServerRedirectTask::newHost() const
{
	return m_newHost;
}

WORD ServerRedirectTask::service() const
{
	return m_service;
}

WORD ServerRedirectTask::chatExchange() const
{
    return m_chatExchange;
}

QString ServerRedirectTask::chatRoomName() const
{
    return m_chatRoom;
}

#include "serverredirecttask.moc"
//kate: indent-mode csands; tab-width 4;

