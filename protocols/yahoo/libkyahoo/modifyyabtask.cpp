/*
    Kopete Yahoo Protocol
    modifyyabtask.h - Handles the Yahoo Address Book

    Copyright (c) 2006 André Duffeck <andre.duffeck@kdemail.net>
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

#include "modifyyabtask.h"
#include "yabtask.h"
#include "transfer.h"
#include "ymsgtransfer.h"
#include "yahootypes.h"
#include "client.h"
#include <qstring.h>
#include <qdatastream.h>
#include <qdom.h>
#include <klocale.h>
#include <kio/global.h>
#include <kio/job.h>
#include <kio/jobclasses.h>
#include <kbufferedsocket.h>

using namespace KNetwork;
ModifyYABTask::ModifyYABTask(Task* parent) : Task(parent)
{
	kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << endl;
	m_socket = 0;
}

ModifyYABTask::~ModifyYABTask()
{
	delete m_socket;
}

void ModifyYABTask::onGo()
{
	kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << endl;
	m_socket = new KBufferedSocket( "address.yahoo.com", QString::number(80) );
	connect( m_socket, SIGNAL( connected( const KResolverEntry& ) ), this, SLOT( connectSucceeded() ) );
	connect( m_socket, SIGNAL( gotError(int) ), this, SLOT( connectFailed(int) ) );

	m_socket->connect();
}

void ModifyYABTask::setAction( Action action )
{
	m_action = action;
}

void ModifyYABTask::setEntry( const YABEntry &entry )
{
	QDomDocument doc("");
	QDomElement root = doc.createElement( "ab" );
	QDomProcessingInstruction instr = doc.createProcessingInstruction("xml","version=\"1.0\" encoding=\"UTF-8\" ");
  	doc.appendChild(instr);
	root.setAttribute( "k", client()->userId() );
	root.setAttribute( "cc", "1" );
	doc.appendChild( root );
	
	QDomElement contact = doc.createElement( "ct" );
	entry.fillQDomElement( contact );
	switch( m_action )
	{
	case EditEntry:
		contact.setAttribute( "e", "1" );
		break;
	case AddEntry:
		contact.setAttribute( "a", "1" );
		break;
	case DeleteEntry:
		contact.setAttribute( "d", "1" );
		break;
	}
	root.appendChild( contact );

	entry.dump();
	m_postData = doc.toString();
}

void ModifyYABTask::connectFailed( int i)
{
	m_socket->close();
	client()->notifyError( i18n( "An error occured saving the Addressbook entry." ), 
			QString( "%1 - %2").arg(i).arg(static_cast<const KBufferedSocket*>( sender() )->errorString()), Client::Error );
}

void ModifyYABTask::connectSucceeded()
{
	kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << endl;

	QString header = QString::fromLatin1("POST /yab/us?v=XM&prog=ymsgr&.intl=us&sync=1&tags=short&noclear=1& HTTP/1.1\r\n"
			"Cookie: Y=%1; T=%2; C=%3 ;B=fckeert1kk1nl&b=2\r\n"
			"User-Agent: Mozilla/4.0 (compatible; MSIE 5.5)\r\n"
			"Host: address.yahoo.com\r\n"
			"Content-length: %4\r\n"
			"Cache-Control: no-cache\r\n\r\n")
			.arg(client()->yCookie()).arg(client()->tCookie())
			.arg(client()->cCookie()).arg(m_postData.utf8().size());

	QByteArray buffer;
	QByteArray paket;
	QDataStream stream( buffer, IO_WriteOnly );
	stream.writeRawBytes( header.local8Bit(), header.length() );
	stream.writeRawBytes( m_postData.utf8(), m_postData.utf8().size() );
	
	if( m_socket->writeBlock( buffer, buffer.size() ) )
		kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << "Upload Successful. Waiting for confirmation..." << endl;
	else
	{
		client()->notifyError( i18n( "An error occured saving the Addressbook entry." ), m_socket->errorString(), Client::Error );
		setSuccess( false );
		return;
	}
	
	connect( m_socket, SIGNAL( readyRead() ), this, SLOT( slotRead() ) );
}

void ModifyYABTask::slotRead()
{
	kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << endl;
	QByteArray ar( m_socket->bytesAvailable() );
	m_socket->readBlock ( ar.data (), ar.size () );
	QString buf( ar );
	m_data += buf.right( buf.length() - buf.find("<?xml") );

	kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << m_data.find("</ab>") << endl;
	if( m_data.find("</ab>") < 0 )
		return;						// Need more data
	kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << m_data.find("</ab>") << endl;

	m_socket->close();
	QDomDocument doc;
	QDomNodeList list;
	QDomElement e;
	uint it = 0;
	
	doc.setContent( m_data );

	list = doc.elementsByTagName( "ab" );			// Get the Addressbook
	for( it = 0; it < list.count(); it++ )	{
		if( !list.item( it ).isElement() )
			continue;
		e = list.item( it ).toElement();
		
		if( !e.attribute( "lm" ).isEmpty() )
			emit gotRevision( e.attribute( "lm" ).toLong(), true );

		if( !e.attribute( "rt" ).isEmpty() )
			emit gotRevision( e.attribute( "rt" ).toLong(), false );
	}

	list = doc.elementsByTagName( "ct" );			// Get records
	for( it = 0; it < list.count(); it++ )	{
		kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << "Parsing entry..." << endl;
		if( !list.item( it ).isElement() )
			continue;
		e = list.item( it ).toElement();
		
		YABEntry *entry = new YABEntry;
		entry->fromQDomElement( e );
		entry->source = YABEntry::SourceYAB;

		switch( m_action )
		{
		case EditEntry:
			if( !e.attribute( "es" ).isEmpty() && e.attribute( "es" ) != "0" )		// Check for edit errors
			{
				emit error( entry, i18n("The Yahoo Addressbook entry could not be saved:\n%1 - %2").arg( e.attribute("es") ).arg( e.attribute("ee") ) );
				continue;
			}
			break;
		case AddEntry:
			if( !e.attribute( "as" ).isEmpty() && e.attribute( "as" ) != "0" )		// Check for add errors
			{
				emit error( entry, i18n("The Yahoo Addressbook entry could not be created:\n%1 - %2").arg( e.attribute("as") ).arg( e.attribute("ae") ) );
				continue;
			}
			break;
		case DeleteEntry:
			if( !e.attribute( "ds" ).isEmpty() && e.attribute( "ds" ) != "0" )		// Check for delete errors
			{
				emit error( entry, i18n("The Yahoo Addressbook entry could not be deleted:\n%1 - %2").arg( e.attribute("ds") ).arg( e.attribute("de") ) );
				continue;
			}
			break;
		}

		// No errors occured
		emit gotEntry( entry );
	}

	
	setSuccess( true );
}
#include "modifyyabtask.moc"
