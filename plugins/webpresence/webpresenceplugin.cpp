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
#include <qdatetime.h>

#include <kdebug.h>
#include <kgenericfactory.h>
#include <ktempfile.h>
#include <kio/job.h>
#include <knotifyclient.h>
#include <kaction.h>
#include <kstdguiitem.h>
#include <kstandarddirs.h>
#include <kglobal.h>
#include <klocale.h>

#include "config.h"
#ifdef HAVE_XSLT
#include <libxml/xmlmemory.h>
#include <libxml/debugXML.h>
#include <libxml/HTMLtree.h>
#include <libxml/xmlIO.h>
#include <libxml/DOCBparser.h>
#include <libxml/xinclude.h>
#include <libxml/catalog.h>
#include <libxslt/xsltconfig.h>
#include <libxslt/xslt.h>
#include <libxslt/xsltInternals.h>
#include <libxslt/transform.h>
#include <libxslt/xsltutils.h>
#endif

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

	kdDebug(14309) << "WebPresencePlugin::slotWriteFile() : " << xml->name() 
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
		kdDebug(14309) << "Error uploading presence info." << endl;
		job->showErrorDialog( 0 );
	}
	delete m_output;
	return;
}

KTempFile* WebPresencePlugin::generateFile()
{
	// generate the (temporary) file representing the current contactlist
	kdDebug( 14309 ) << "WebPresencePlugin::generateFile()" << endl;

	KTempFile* theFile = new KTempFile();
	QTextStream* qout =  theFile->textStream() ;
	QString output;
	QString notKnown = i18n( "Not yet known" );
	QPtrList<KopeteProtocol> protocols = allProtocols();

	XMLHelper h;
	output += h.content( "<?xml version=\"1.0\"?>" );
	output += h.openTag( "contacts" );

	// insert the current date/time
	output += h.oneLineTag( "listdate",
			KGlobal::locale()->formatDateTime( QDateTime::currentDateTime() ) );
			
	output += h.openTag( "contact", "type=\"self\"" );

	// insert the contact's name
	if ( m_prefs->useImName() && !m_prefs->userName().isEmpty() )
		output += h.oneLineTag( "name",  m_prefs->userName() );
	else
	{
		// pick the name from the first configured protocol
		QString myName;
		for ( KopeteProtocol *p = protocols.first(); p; p = protocols.next() )
			if ( p->myself() != 0L )
			{
				myName = p->myself()->displayName();
				break;
			}
		if ( myName.isNull() )
			myName = i18n( "No Name Set Yet, configure your IM protocols and connect!" );
		output += h.oneLineTag( "name",  myName );
	}

	// insert the list of the contact's protocols
	output += h.openTag( "protocols" );

	for ( KopeteProtocol *p = protocols.first();
			p; p = protocols.next() )
	{
		KopeteContact* me = p->myself();

		output += h.openTag( "protocol" );

		output += h.oneLineTag( "protoname", p->pluginId() );

		output += h.oneLineTag( "protostatus",
				( me )
				? statusAsString( me->onlineStatus() )
				: notKnown );

		if ( m_prefs->showAddresses() )
			output += h.oneLineTag( "protoaddress",
					( me )
					? me->contactId().latin1()
					: notKnown.latin1()
					);

		output += h.closeTag();
	}

	// finish off neatly
	output += h.closeTag();
	output += h.closeTag();
	output += h.closeTag();

	// write our XML
	*qout << output;
	
	theFile->close();
	return theFile;
}

bool WebPresencePlugin::transform( KTempFile* src, KTempFile* dest )
{
#ifdef HAVE_XSLT
	QString error = "";
	xmlSubstituteEntitiesDefault( 1 );
	xmlLoadExtDtdDefaultValue = 1;
	// test if the stylesheet exists
	QFile sheet;
	if ( m_prefs->useDefaultStyleSheet() )
		sheet.setName( locate( "appdata", "webpresencedefault.xsl" ) );
	else
		sheet.setName( m_prefs->userStyleSheet() );
	
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
		kdDebug(14309) << "WebPresencePlugin::transform() - couldn't "
			<< error << endl;
		return false;
	}
#else
	return false;
#endif
}

QPtrList<KopeteProtocol> WebPresencePlugin::allProtocols()
{
	kdDebug(14309) << "WebPresencePlugin::allProtocols()" << endl;
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

QString WebPresencePlugin::statusAsString( const KopeteOnlineStatus &newStatus )
{
	QString status;
	switch ( newStatus.status() )
	{
	case KopeteOnlineStatus::Online:
		status = "ONLINE";
		break;
	case KopeteOnlineStatus::Away:
		status = "AWAY";
		break;
	case KopeteOnlineStatus::Offline:
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

WebPresencePlugin::XMLHelper::XMLHelper()
{
	depth = 0;
	stack = new QValueStack<QString>();
}

WebPresencePlugin::XMLHelper::~XMLHelper()
{
	delete stack;
}

QString WebPresencePlugin::XMLHelper::oneLineTag( QString name, QString content, QString attrs )
{
	QString out;
	out.fill( '\t', depth );
	out += "<" + name;
	if ( !attrs.isEmpty() )
		out += " " + attrs;
	if ( !content.isEmpty() )
		out += ">" + content + "</" + name + ">\n";
	else
		out += "/>\n";
	return out;
}

QString WebPresencePlugin::XMLHelper::openTag( QString name, QString attrs )
{
	QString out;
	out.fill( '\t', depth++ );
	out += "<" + name;
	if ( !attrs.isEmpty() )
		out += " " + attrs;
	out += ">\n";

	stack->push( name );

	return out;
}

QString WebPresencePlugin::XMLHelper::content( QString content )
{
	QString out;
	out.fill( '\t', depth );
	out += content + "\n";

	return out;
}

QString WebPresencePlugin::XMLHelper::closeTag()
{
	QString out;
	out.fill(  '\t', --depth );
	out += "</" + stack->pop () + ">\n";

	return out;
}

QString WebPresencePlugin::XMLHelper::closeAll()
{
	QString out;
	while ( !stack->isEmpty() )
	{
		out.fill(   '\t', --depth );
		out += "</" + stack->pop () + ">\n";
	}

	return out;
}

// vim: set noet ts=4 sts=4 sw=4:
#include "webpresenceplugin.moc"
