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
#include <qfile.h>

#include <kdebug.h>
#include <kgenericfactory.h>
#include <ktempfile.h>
#include <kio/job.h>
#include <knotifyclient.h>
#include <kaction.h>
#include <kstdguiitem.h>
#include <kstandarddirs.h>

#include <libxml/xmlmemory.h>
#include <libxml/debugXML.h>
#include <libxml/HTMLtree.h>
#include <libxml/xmlIO.h>
#include <libxml/DOCBparser.h>
#include <libxml/xinclude.h>
#include <libxml/catalog.h>
#include <libxslt/xslt.h>
#include <libxslt/xsltInternals.h>
#include <libxslt/transform.h>
#include <libxslt/xsltutils.h>

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
	m_prefs = new WebPresencePreferences( "", this );
	connect ( m_prefs, SIGNAL( saved() ), this, SLOT( slotSettingsChanged() ) );
	m_timer = new QTimer();
	connect ( m_timer, SIGNAL( timeout() ), this, SLOT( slotWriteFile() ) );
	m_timer->start( m_prefs->frequency() * 1000 * 60);
}

WebPresencePlugin::~WebPresencePlugin()
{}

void WebPresencePlugin::slotWriteFile()
{
	bool error = false;
	// generate the (temporary) XML file representing the current contactlist
	KTempFile* xml = generateFile();
	xml->setAutoDelete( true );
	
	kdDebug() << "WebPresencePlugin::slotWriteFile() : " << xml->name() 
		<< endl;
	
	if ( m_prefs->justXml() )
	{
	    m_output = xml;
		xml = 0L;
	}
	else
	{
		// transform XML to the final format
		m_output = new KTempFile();
		m_output->setAutoDelete( true );
		if ( !transform( xml, m_output ) )
		{
			error = true;
			delete m_output;
		}
		delete xml; // might make debugging harder!
	}
	if ( !error )
	{
		// upload it to the specified URL
		KURL src( m_output->name() );
		KURL dest( m_prefs->url() );
		KIO::FileCopyJob *job = KIO::file_copy( src, dest, -1, true, false, false );
		connect( job, SIGNAL( result( KIO::Job * ) ),
				SLOT(  slotUploadJobResult( KIO::Job * ) ) );
	}
}

void WebPresencePlugin::slotUploadJobResult( KIO::Job *job )
{
	if (  job->error() ) {
		kdDebug() << "Error uploading presence info." << endl;
		job->showErrorDialog( 0 );
	}
	delete m_output;
	return;
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
	if ( !m_prefs->useImName() && !m_prefs->userName().isEmpty() )
		*qout << m_prefs->userName();
	else
		*qout << protocols.first()->myself()->displayName();
	*qout << "</name>\n";
	
	*qout << shift.fill( '\t', depth++ ) << "<protocols>\n";
	for ( KopeteProtocol *p = protocols.first();
			p; p = protocols.next() )
	{
		KopeteContact* me = p->myself();
		QString notKnown = i18n( "Not yet known" );

		*qout << shift.fill( '\t', depth++ ) << "<protocol>\n";
		
		*qout << shift.fill( '\t', depth ) << "<protoname>";
		*qout << p->pluginId();
		*qout << "</protoname>";
		
		*qout << shift.fill( '\t', depth ) << "<protostatus>";
		if ( me )
			*qout << statusAsString( me->status() );
		else
			*qout << notKnown;
		*qout << "</protostatus>\n";

		if ( m_prefs->showAddresses() )
		{
			*qout << shift.fill( '\t', depth ) << "<protoaddress>";
			if ( me )
				*qout << me->contactId().latin1();
			else
				*qout << notKnown;
			*qout << "</protoaddress>\n";
		}
		*qout << shift.fill( '\t', --depth ) << "</protocol>\n";
	}
	*qout << shift.fill( '\t', --depth ) << "</protocols>\n";
	*qout << shift.fill( '\t', --depth ) << "</contact>\n";

	*qout << shift.fill( '\t', --depth ) << "</contacts>\n" << endl;

	theFile->close();
	return theFile;
}

bool WebPresencePlugin::transform( KTempFile* src, KTempFile* dest )
{
	xmlSubstituteEntitiesDefault( 1 );
	xmlLoadExtDtdDefaultValue = 1;
	// test if the stylesheet exists
	QFile sheet;
	if ( m_prefs->useDefaultStyleSheet() )
		sheet.setName( locate( "appdata", "webpresencedefault.xsl" ) );
	else
		sheet.setName( m_prefs->userStyleSheet() );
	
	QString error = "";
	if ( sheet.exists() )
	{
		// and if it is a valid stylesheet
		xsltStylesheetPtr cur = NULL;
		if ( ( cur = xsltParseStylesheetFile( 
					( const xmlChar *) sheet.name().latin1() ) ) )
		{
			// and if we can parse the input XML
			xmlDocPtr doc = NULL;
			if ( ( doc = xmlParseFile( src->name() ) ) )
			{
				// and if we can apply the stylesheet
				xmlDocPtr res = NULL;
				if ( ( res = xsltApplyStylesheet( cur, doc, 0 ) ) )
				{
					// and if we can save the result
					if ( xsltSaveResultToFile(dest->fstream() , res, cur)
							!= -1 )
					{
						// then it all worked!
						dest->close();
					}
					else
						error = "write result!";
				}
				else
				{
					error = "apply stylesheet!";
					error += " Check the stylesheet works using xsltproc";
				}
				xmlFreeDoc(res);
			}
			else
				error = "parse input XML!";
			xmlFreeDoc(doc);
		}
		else
			error = "parse stylesheet!";
		xsltFreeStylesheet(cur);
	}
	else
		error = "find stylesheet" + sheet.name() + "!";

	xsltCleanupGlobals();
	xmlCleanupParser();
	
	if ( error.isEmpty() )
		return true;
	else
	{
		kdDebug() << "WebPresencePlugin::transform() - couldn't "
			<< error << endl;
		return false;
	}
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
	m_timer->start( m_prefs->frequency() * 1000 * 60);
}


// vim: set noet ts=4 sts=4 sw=4:
#include "webpresenceplugin.moc"
