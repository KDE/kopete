/*
   webpresenceplugin.cpp

   Kopete Web Presence plugin

   Copyright (c) 2002 by Will Stephenson <will@stevello.free-online.co.uk>

   Kopete    (c) 2002 by the Kopete developers  <kopete-devel@kde.org>

 *************************************************************************
 *                                                                    	*
 * This program is free software; you can redistribute it and/or modify  *
 * it under the terms of the GNU General Public License as published by  *
 * the Free Software Foundation; either version 2 of the License, or     *
 * (at your option) any later version.                                   *
 *                                                                       *
 *************************************************************************
 */

#include <qtimer.h>
#include <qptrlist.h>

#include <kdebug.h>
#include <kgenericfactory.h>
#include <ktempfile.h>
#include <kio/job.h>
#include <knotifyclient.h>
#include <kaction.h>
#include <kstdguiitem.h>

#include "kopetecontactlist.h"
#include "pluginloader.h"
#include "kopeteprotocol.h"
#include "kopetemetacontact.h"

#include "webpresenceplugin.h"
#include "webpresencepreferences.h"

K_EXPORT_COMPONENT_FACTORY( kopete_webpresence, KGenericFactory<WebPresencePlugin> );

	WebPresencePlugin::WebPresencePlugin( QObject *parent, const char *name, const QStringList& /*args*/ )
: KopetePlugin( parent, name )
{
	m_prefsDialog = new WebPresencePreferences( "", this );
	connect ( m_prefsDialog, SIGNAL( saved() ), this, SLOT( slotSettingsChanged() ) );
	m_timer = new QTimer();
	connect ( m_timer, SIGNAL( timeout() ), this, SLOT( slotWriteFile() ) );
	m_timer->start( m_prefsDialog->frequency() * 1000 * 60);
}

WebPresencePlugin::~WebPresencePlugin()
{}

void WebPresencePlugin::slotWriteFile()
{
	// generate the (temporary) file representing the current contactlist
	KTempFile* theFile = generateFile();
	theFile->setAutoDelete( false );
	kdDebug() << "WebPresencePlugin::slotWriteFile() : " << theFile->name() << endl;
	// upload it to the specified URL
	KURL src( theFile->name() );
	KURL dest( m_prefsDialog->url() );
	KIO::FileCopyJob *job = KIO::file_copy( src, dest, -1, true, false, false );
	connect(  job, SIGNAL(  result(  KIO::Job * ) ),
			SLOT(  slotUploadJobResult(  KIO::Job * ) ) );
	// get rid of the local file
	delete theFile;
}

void WebPresencePlugin::slotUploadJobResult( KIO::Job *job )
{
	if (  job->error() ) {
		kdDebug() << "Error uploading presence info." << endl;
		job->showErrorDialog( 0 );
		return;
	}
}

KTempFile* WebPresencePlugin::generateFile()
{
	kdDebug() << "WebPresencePlugin::generateFile()" << endl;
	// generate the (temporary) file representing the current contactlist
	KTempFile* theFile = new KTempFile();
	QTextStream* qout =  theFile->textStream() ;
	QPtrList<KopeteProtocol> protocols = allProtocols();

	int depth = 0;
	QString shift;
	*qout << "<?xml version=\"1.0\"?>\n" 
		<< shift.fill( '\t', ++depth ) << "<contacts>\n";

	*qout << shift.fill( '\t', ++depth ) << "<contact type=\"self\">\n";
	
	*qout << shift.fill( '\t', ++depth ) << "<name>";
	if ( !m_prefsDialog->useImName() && !m_prefsDialog->userName().isEmpty() )
		*qout << m_prefsDialog->userName();
	else
		*qout << protocols.first()->myself()->displayName();
	*qout << "</name>\n";
	
	*qout << shift.fill( '\t', depth++ ) << "<protocols>\n";
	for ( KopeteProtocol *p = protocols.first();
			p; p = protocols.next() )
	{
		*qout << shift.fill( '\t', depth++ ) << "<protocol>\n";
		*qout << shift.fill( '\t', depth ) << "<protoname>"
			<< p->pluginId() << "</protoname>\n";
		*qout << shift.fill( '\t', depth ) << "<protostatus>"
			<< statusAsString( p->myself()->status() )
			<< "</protostatus>\n";
		if ( m_prefsDialog->showAddresses() )
		{
			*qout << shift.fill( '\t', depth ) << "<protoaddress>"
				<< p->myself()->contactId().latin1()
				<< "</protoaddress>\n";
		}
		*qout << shift.fill( '\t', --depth ) << "</protocol>\n";
	}
	*qout << shift.fill( '\t', --depth ) << "</protocols>\n";
	*qout << shift.fill( '\t', --depth ) << "</contact>\n";

	*qout << shift.fill( '\t', --depth ) << "</contacts>\n" << endl;

	theFile->close();
	return theFile;
}

QPtrList<KopeteProtocol> WebPresencePlugin::allProtocols()
{
	kdDebug() << "WebPresencePlugin::allProtocols()" << endl;
	QPtrList<KopeteProtocol> protos;
	QPtrList<KopetePlugin> plugins = LibraryLoader::pluginLoader()->plugins();

	for( KopetePlugin *p = plugins.first(); p; p = plugins.next() )
	{
		KopeteProtocol *proto = dynamic_cast<KopeteProtocol*>( p );
		if( !proto )
			continue;
		protos.append( proto );
	}
	return protos;
}

QString WebPresencePlugin::statusAsString( KopeteContact::ContactStatus c )
{
	QString status;
	switch ( c )
	{
		case KopeteContact::Online:
			status = "ONLINE";
			break;
		case KopeteContact::Away:
			status = "AWAY";
			break;
		case KopeteContact::Offline:
			status = "OFFLINE";
			break;
		default:
			status = "UNKNOWN";
	}
	return status;
}

void WebPresencePlugin::slotSettingsChanged()
{
	m_timer->start( m_prefsDialog->frequency() * 1000 * 60);
}

//#include "webpresenceplugin.moc"

// vim: set noet ts=4 sts=4 sw=4:
#include "webpresenceplugin.moc"
