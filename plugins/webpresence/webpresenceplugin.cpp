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

#include "config.h"

#include <qtimer.h>
#include <qfile.h>

#include <kdebug.h>
#include <kgenericfactory.h>
#include <ktempfile.h>
#include <kstandarddirs.h>

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

#include "pluginloader.h"
#include "kopeteprotocol.h"
#include "kopeteaccountmanager.h"
#include "kopeteaccount.h"

#include "webpresenceplugin.h"
#include "webpresencepreferences.h"

K_EXPORT_COMPONENT_FACTORY( kopete_webpresence, KGenericFactory<WebPresencePlugin> );

WebPresencePlugin::WebPresencePlugin( QObject *parent, const char *name, const QStringList& /*args*/ )
: KopetePlugin( parent, name )
{
	m_writeScheduler = new QTimer( this );
	connect ( m_writeScheduler, SIGNAL( timeout() ), this, SLOT( slotWriteFile() ) );
	m_prefs = new WebPresencePreferences( "", this );
	connect( KopeteAccountManager::manager(), SIGNAL(accountReady(KopeteAccount*)),
				this, SLOT( listenToAllAccounts() ) );
	connect( KopeteAccountManager::manager(), SIGNAL(accountUnregistered(KopeteAccount*)),
				this, SLOT( listenToAllAccounts() ) );

}

WebPresencePlugin::~WebPresencePlugin()
{
}

void WebPresencePlugin::listenToAllAccounts()
{
	// connect to signals notifying of all accounts' status changes
	QPtrList<KopeteProtocol> protocols = allProtocols();
	for ( KopeteProtocol *p = protocols.first();
			p; p = protocols.next() )
	{
		QDict<KopeteAccount> dict=KopeteAccountManager::manager()->accounts( p );
		QDictIterator<KopeteAccount> it( dict );
		for( ; KopeteAccount *account=it.current(); ++it )
		{
			 listenToAccount( account );
		}
 	}
	slotWaitMoreStatusChanges();
}

void WebPresencePlugin::listenToAccount( KopeteAccount* account )
{
	if(account->myself())
	{
		// Connect to the account's status changed signal
		// because we can't know if the account has already connected
		QObject::disconnect( account->myself(),
						SIGNAL(onlineStatusChanged( KopeteContact *,
								const KopeteOnlineStatus &,
								const KopeteOnlineStatus & ) ),
						this,
						SLOT( slotWaitMoreStatusChanges() ) ) ;
		QObject::connect( account->myself(),
						SIGNAL(onlineStatusChanged( KopeteContact *,
								const KopeteOnlineStatus &,
								const KopeteOnlineStatus & ) ),
						this,
						SLOT( slotWaitMoreStatusChanges() ) );
	}
}

void WebPresencePlugin::slotWaitMoreStatusChanges()
{
	if ( !m_writeScheduler->isActive() )
		 m_writeScheduler->start( m_prefs->frequency() * 1000);

}

void WebPresencePlugin::slotWriteFile()
{
	bool error = false;
	// generate the (temporary) XML file representing the current contactlist
	KTempFile* xml = generateFile();
	xml->setAutoDelete( true );

	kdDebug(14309) << k_funcinfo << " " << xml->name() << endl;

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
	m_writeScheduler->stop();
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
	kdDebug( 14309 ) << k_funcinfo << endl;

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

	// insert the user's name
	if ( !m_prefs->useImName() && !m_prefs->userName().isEmpty() )
		output += h.oneLineTag( "name", m_prefs->userName() );
	else
		output += h.oneLineTag( "name", notKnown );

	// insert the list of the contact's protocols
	output += h.openTag( "protocols" );

	for ( KopeteProtocol *p = protocols.first();
			p; p = protocols.next() )
	{
		// get all the accounts per protocol
		QDict<KopeteAccount> dict = KopeteAccountManager::manager()->accounts( p );
		// If no accounts, stop here
		if ( dict.isEmpty() )
			continue;

		output += h.openTag( "protocol" );
		output += h.oneLineTag( "protoname", p->pluginId() );

		for( QDictIterator<KopeteAccount> it( dict );
			 KopeteAccount *account=it.current();
			 ++it )
		{
			output += h.openTag( "account" );

			KopeteContact* me = account->myself();
			output += h.oneLineTag( "accountname",
					( me )
					? me->displayName().latin1()
					: notKnown.latin1() );
			output += h.oneLineTag( "accountstatus",
					( me )
					? statusAsString( me->onlineStatus() )
					: notKnown );

			if ( m_prefs->showAddresses() )
				output += h.oneLineTag( "accountaddress",
						( me )
						? me->contactId().latin1()
						: notKnown.latin1() );

			output += h.closeTag();
		}
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

bool WebPresencePlugin::transform( KTempFile * src, KTempFile * dest )
{
#ifdef HAVE_XSLT
	QString error = "";
	xmlSubstituteEntitiesDefault( 1 );
	xmlLoadExtDtdDefaultValue = 1;
	// test if the stylesheet exists
	QFile sheet;
	if ( m_prefs->useDefaultStyleSheet() )
		sheet.setName( locate( "appdata", "webpresence/webpresencedefault.xsl" ) );
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
		kdDebug(14309) << k_funcinfo << " - couldn't "
			<< error << endl;
		return false;
	}
#else
	Q_UNUSED( src );
	Q_UNUSED( dest );

	return false;
#endif
}

QPtrList<KopeteProtocol> WebPresencePlugin::allProtocols()
{
	kdDebug(14309) << k_funcinfo << endl;
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
