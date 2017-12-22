/*
    Kopete Yahoo Protocol
    modifyyabtask.h - Handles the Yahoo Address Book

    Copyright (c) 2006 André Duffeck <duffeck@kde.org>
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
#include <k3bufferedsocket.h>

using namespace KNetwork;
using namespace KYahoo;

ModifyYABTask::ModifyYABTask(Task* parent) : Task(parent)
{
	kDebug(YAHOO_RAW_DEBUG) ;
	m_socket = 0;
}

ModifyYABTask::~ModifyYABTask()
{
	delete m_socket;
}

void ModifyYABTask::onGo()
{
	kDebug(YAHOO_RAW_DEBUG) ;
	m_socket = new KBufferedSocket( QStringLiteral("address.yahoo.com"), QString::number(80) );
	connect( m_socket, SIGNAL(connected(KNetwork::KResolverEntry)), this, SLOT(connectSucceeded()) );
	connect( m_socket, SIGNAL(gotError(int)), this, SLOT(connectFailed(int)) );

	m_socket->connect();
}

void ModifyYABTask::setAction( Action action )
{
	m_action = action;
}

void ModifyYABTask::setEntry( const YABEntry &entry )
{
	QDomDocument doc(QLatin1String(""));
	QDomElement root = doc.createElement( QStringLiteral("ab") );
	QDomProcessingInstruction instr = doc.createProcessingInstruction(QStringLiteral("xml"),QStringLiteral("version=\"1.0\" encoding=\"UTF-8\" "));
  	doc.appendChild(instr);
	root.setAttribute( QStringLiteral("k"), client()->userId() );
	root.setAttribute( QStringLiteral("cc"), QStringLiteral("1") );
	doc.appendChild( root );
	
	QDomElement contact = doc.createElement( QStringLiteral("ct") );
	entry.fillQDomElement( contact );
	switch( m_action )
	{
	case EditEntry:
		contact.setAttribute( QStringLiteral("e"), QStringLiteral("1") );
		break;
	case AddEntry:
		contact.setAttribute( QStringLiteral("a"), QStringLiteral("1") );
		break;
	case DeleteEntry:
		contact.setAttribute( QStringLiteral("d"), QStringLiteral("1") );
		break;
	}
	root.appendChild( contact );

	entry.dump();
	m_postData = doc.toString();
}

void ModifyYABTask::connectFailed( int i)
{
	m_socket->close();
	client()->notifyError( i18n( "An error occurred while saving the address book entry." ), 
			QStringLiteral( "%1 - %2").arg(i).arg(static_cast<const KBufferedSocket*>( sender() )->errorString()), Client::Error );
}

void ModifyYABTask::connectSucceeded()
{
	kDebug(YAHOO_RAW_DEBUG) ;
	KBufferedSocket* socket = const_cast<KBufferedSocket*>( static_cast<const KBufferedSocket*>( sender() ) );

	QString header = QString::fromLatin1("POST /yab/us?v=XM&prog=ymsgr&.intl=us&sync=1&tags=short&noclear=1& HTTP/1.1\r\n"
			"Cookie: Y=%1; T=%2; C=%3 ;B=fckeert1kk1nl&b=2\r\n"
			"User-Agent: Mozilla/4.0 (compatible; MSIE 5.5)\r\n"
			"Host: address.yahoo.com\r\n"
			"Content-length: %4\r\n"
			"Cache-Control: no-cache\r\n\r\n")
			.arg(client()->yCookie()).arg(client()->tCookie())
			.arg(client()->cCookie()).arg(m_postData.toUtf8().size());

	QByteArray buffer;
	QByteArray paket;
	QDataStream stream( &buffer, QIODevice::WriteOnly );
	stream.writeRawData( header.toLocal8Bit(), header.length() );
	stream.writeRawData( m_postData.toUtf8(), m_postData.toUtf8().size() );
	
	if( socket->write( buffer, buffer.size() ) )
		kDebug(YAHOO_RAW_DEBUG) << "Upload Successful. Waiting for confirmation...";
	else
	{
		client()->notifyError( i18n( "An error occurred while saving the address book entry." ), m_socket->errorString(), Client::Error );
		setError();
		return;
	}
	
	connect( m_socket, SIGNAL(readyRead()), this, SLOT(slotRead()) );
}

void ModifyYABTask::slotRead()
{
	KBufferedSocket* socket = const_cast<KBufferedSocket*>( static_cast<const KBufferedSocket*>( sender() ) );
	QByteArray ar;
	ar.reserve( socket->bytesAvailable() );
	socket->read( ar.data (), ar.size () );
	QString data( ar );
	data = data.right( data.length() - data.indexOf(QLatin1String("<?xml")) );

	if( m_data.indexOf(QLatin1String("</ab>")) < 0 )
		return;						// Need more data

	m_socket->close();
	QDomDocument doc;
	QDomNodeList list;
	QDomElement e;
	int it = 0;
	
	doc.setContent( m_data );

	list = doc.elementsByTagName( QStringLiteral("ab") );			// Get the Addressbook
	for( it = 0; it < list.count(); it++ )	{
		if( !list.item( it ).isElement() )
			continue;
		e = list.item( it ).toElement();
		
		if( !e.attribute( QStringLiteral("lm") ).isEmpty() )
			emit gotRevision( e.attribute( QStringLiteral("lm") ).toLong(), true );

		if( !e.attribute( QStringLiteral("rt") ).isEmpty() )
			emit gotRevision( e.attribute( QStringLiteral("rt") ).toLong(), false );
	}

	list = doc.elementsByTagName( QStringLiteral("ct") );			// Get records
	for( it = 0; it < list.count(); it++ )	{
		kDebug(YAHOO_RAW_DEBUG) << "Parsing entry...";
		if( !list.item( it ).isElement() )
			continue;
		e = list.item( it ).toElement();
		
		YABEntry *entry = new YABEntry;
		entry->fromQDomElement( e );
		entry->source = YABEntry::SourceYAB;

		switch( m_action )
		{
		case EditEntry:
			if( !e.attribute( QStringLiteral("es") ).isEmpty() && e.attribute( QStringLiteral("es") ) != QLatin1String("0") )		// Check for edit errors
			{
				emit error( entry, i18n("The Yahoo Address Book entry could not be saved:\n%1 - %2", e.attribute(QStringLiteral("es")), e.attribute(QStringLiteral("ee")) ) );
				continue;
			}
			break;
		case AddEntry:
			if( !e.attribute( QStringLiteral("as") ).isEmpty() && e.attribute( QStringLiteral("as") ) != QLatin1String("0") )		// Check for add errors
			{
				emit error( entry, i18n("The Yahoo Address Book entry could not be created:\n%1 - %2", e.attribute(QStringLiteral("as")), e.attribute(QStringLiteral("ae")) ) );
				continue;
			}
			break;
		case DeleteEntry:
			if( !e.attribute( QStringLiteral("ds") ).isEmpty() && e.attribute( QStringLiteral("ds") ) != QLatin1String("0") )		// Check for delete errors
			{
				emit error( entry, i18n("The Yahoo Address Book entry could not be deleted:\n%1 - %2", e.attribute(QStringLiteral("ds")), e.attribute(QStringLiteral("de")) ) );
				continue;
			}
			break;
		}

		// No errors occurred
		emit gotEntry( entry );
	}

	
	setSuccess();
}
