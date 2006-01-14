/*
   webpresenceplugin.cpp

   Kopete Web Presence plugin

   Copyright (c) 2005      by Tommi Rantala   <tommi.rantala@cs.helsinki.fi>
   Copyright (c) 2002,2003 by Will Stephenson <will@stevello.free-online.co.uk>

   Kopete    (c) 2002-2005 by the Kopete developers  <kopete-devel@kde.org>

 *************************************************************************
 *                                                                    	 *
 * This program is free software; you can redistribute it and/or modify  *
 * it under the terms of the GNU General Public License as published by  *
 * the Free Software Foundation; either version 2 of the License, or     *
 * (at your option) any later version.                                   *
 *                                                                       *
 *************************************************************************
 */

#include "config.h"

#include <qdom.h>
#include <qtimer.h>
#include <qfile.h>

#include <kdebug.h>
#include <kconfig.h>
#include <kgenericfactory.h>
#include <kmessagebox.h>
#include <ktempfile.h>
#include <kstandarddirs.h>

#ifdef HAVE_XSLT
#include <libxml/parser.h>
#include <libxml/tree.h>

#include <libxslt/xsltconfig.h>
#include <libxslt/xsltInternals.h>
#include <libxslt/transform.h>
#include <libxslt/xsltutils.h>
#endif

#include "kopetepluginmanager.h"
#include "kopeteprotocol.h"
#include "kopeteaccountmanager.h"
#include "kopeteaccount.h"

#include "webpresenceplugin.h"

typedef KGenericFactory<WebPresencePlugin> WebPresencePluginFactory;
K_EXPORT_COMPONENT_FACTORY( kopete_webpresence, WebPresencePluginFactory( "kopete_webpresence" )  )

WebPresencePlugin::WebPresencePlugin( QObject *parent, const char *name, const QStringList& /*args*/ )
	: Kopete::Plugin( WebPresencePluginFactory::instance(), parent, name ),
	shuttingDown( false ), resultFormatting( WEB_HTML )
{
	m_writeScheduler = new QTimer( this );
	connect ( m_writeScheduler, SIGNAL( timeout() ), this, SLOT( slotWriteFile() ) );
	connect( Kopete::AccountManager::self(), SIGNAL(accountRegistered(Kopete::Account*)),
				this, SLOT( listenToAllAccounts() ) );
	connect( Kopete::AccountManager::self(), SIGNAL(accountUnregistered(Kopete::Account*)),
				this, SLOT( listenToAllAccounts() ) );

	connect(this, SIGNAL(settingsChanged()), this, SLOT( loadSettings() ) );
	loadSettings();
	listenToAllAccounts();
}

WebPresencePlugin::~WebPresencePlugin()
{
}

void WebPresencePlugin::loadSettings()
{
	KConfig *kconfig = KGlobal::config();
	kconfig->setGroup( "Web Presence Plugin" );

	frequency = kconfig->readNumEntry("UploadFrequency", 15);
	resultURL = kconfig->readPathEntry("uploadURL");

	resultFormatting = WEB_UNDEFINED;

	if ( kconfig->readBoolEntry( "formatHTML", false ) ) {
		resultFormatting = WEB_HTML;
	} else if ( kconfig->readBoolEntry( "formatXHTML", false ) ) {
		resultFormatting = WEB_XHTML;
	} else if ( kconfig->readBoolEntry( "formatXML", false ) ) {
		resultFormatting = WEB_XML;
	} else if ( kconfig->readBoolEntry( "formatStylesheet", false ) ) {
		resultFormatting = WEB_CUSTOM;
		userStyleSheet = kconfig->readEntry("formatStylesheetURL");
	}

	// Default to HTML if we dont get anything useful from config file.
	if ( resultFormatting == WEB_UNDEFINED )
		resultFormatting = WEB_HTML;

	useImagesInHTML = kconfig->readBoolEntry( "useImagesHTML", false );
	useImName = kconfig->readBoolEntry("showName", true);
	userName = kconfig->readEntry("showThisName");
	showAddresses = kconfig->readBoolEntry("includeIMAddress", false);

	// Update file when settings are changed.
	slotWriteFile();
}

void WebPresencePlugin::listenToAllAccounts()
{
	// connect to signals notifying of all accounts' status changes
	ProtocolList protocols = allProtocols();

	for ( ProtocolList::Iterator it = protocols.begin();
			it != protocols.end(); ++it )
	{
		QDict<Kopete::Account> accounts = Kopete::AccountManager::self()->accounts( *it );
		QDictIterator<Kopete::Account> acIt( accounts );

		for( ; Kopete::Account *account = acIt.current(); ++acIt )
		{
			listenToAccount( account );
		}
	}
	slotWaitMoreStatusChanges();
}

void WebPresencePlugin::listenToAccount( Kopete::Account* account )
{
	if(account && account->myself())
	{
		// Connect to the account's status changed signal
		// because we can't know if the account has already connected
		QObject::disconnect( account->myself(),
						SIGNAL(onlineStatusChanged( Kopete::Contact *,
								const Kopete::OnlineStatus &,
								const Kopete::OnlineStatus & ) ),
						this,
						SLOT( slotWaitMoreStatusChanges() ) ) ;
		QObject::connect( account->myself(),
						SIGNAL(onlineStatusChanged( Kopete::Contact *,
								const Kopete::OnlineStatus &,
								const Kopete::OnlineStatus & ) ),
						this,
						SLOT( slotWaitMoreStatusChanges() ) );
	}
}

void WebPresencePlugin::slotWaitMoreStatusChanges()
{
	if ( !m_writeScheduler->isActive() )
		m_writeScheduler->start( frequency * 1000 );
}

void WebPresencePlugin::slotWriteFile()
{
	m_writeScheduler->stop();

	// generate the (temporary) XML file representing the current contactlist
	KURL dest( resultURL );
	if ( resultURL.isEmpty() || !dest.isValid() )
	{
		kdDebug(14309) << "url is empty or not valid. NOT UPDATING!" << endl;
		return;
	}

	KTempFile* xml = generateFile();
	xml->setAutoDelete( true );
	kdDebug(14309) << k_funcinfo << " " << xml->name() << endl;

	switch( resultFormatting ) {
	case WEB_XML:
		m_output = xml;
		xml = 0L;
		break;
	case WEB_HTML:
	case WEB_XHTML:
	case WEB_CUSTOM:
		m_output = new KTempFile();
		m_output->setAutoDelete( true );

		if ( !transform( xml, m_output ) )
		{
			//TODO: give some error to user, even better if shown only once
			delete m_output;
			m_output = 0L;

			delete xml;
			return;
		}

		delete xml; // might make debugging harder!
		break;
	default:
		return;
	}

	// upload it to the specified URL
	KURL src( m_output->name() );
	KIO::FileCopyJob *job = KIO::file_move( src, dest, -1, true, false, false );
	connect( job, SIGNAL( result( KIO::Job * ) ),
			SLOT(  slotUploadJobResult( KIO::Job * ) ) );
}

void WebPresencePlugin::slotUploadJobResult( KIO::Job *job )
{
	if ( job->error() ) {
		kdDebug(14309) << "Error uploading presence info." << endl;
		KMessageBox::queuedDetailedError( 0, i18n("An error occurred when uploading your presence page.\nCheck the path and write permissions of the destination."), 0, displayName() );
		delete m_output;
		m_output = 0L;
	}
}

KTempFile* WebPresencePlugin::generateFile()
{
	// generate the (temporary) XML file representing the current contactlist
	kdDebug( 14309 ) << k_funcinfo << endl;
	QString notKnown = i18n( "Not yet known" );

	QDomDocument doc;

	doc.appendChild( doc.createProcessingInstruction( "xml",
				"version=\"1.0\" encoding=\"UTF-8\"" ) );

	QDomElement root = doc.createElement( "webpresence" );
	doc.appendChild( root );

	// insert the current date/time
	QDomElement date = doc.createElement( "listdate" );
	QDomText t = doc.createTextNode( 
			KGlobal::locale()->formatDateTime( QDateTime::currentDateTime() ) );
	date.appendChild( t );
	root.appendChild( date );

	// insert the user's name
	QDomElement name = doc.createElement( "name" );
	QDomText nameText;
	if ( !useImName && !userName.isEmpty() )
		nameText = doc.createTextNode( userName );
	else
		nameText = doc.createTextNode( notKnown );
	name.appendChild( nameText );
	root.appendChild( name );

	// insert the list of the user's accounts
	QDomElement accounts = doc.createElement( "accounts" );
	root.appendChild( accounts );

	QPtrList<Kopete::Account> list = Kopete::AccountManager::self()->accounts();
	// If no accounts, stop here
	if ( !list.isEmpty() )
	{
		for( QPtrListIterator<Kopete::Account> it( list );
			 Kopete::Account *account=it.current();
			 ++it )
		{
			QDomElement acc = doc.createElement( "account" );
			//output += h.openTag( "account" );

			QDomElement protoName = doc.createElement( "protocol" );
			QDomText protoNameText = doc.createTextNode(
					account->protocol()->pluginId() );
			protoName.appendChild( protoNameText );
			acc.appendChild( protoName );

			Kopete::Contact* me = account->myself();
			QString displayName = me->property( Kopete::Global::Properties::self()->nickName() ).value().toString();
			QDomElement accName = doc.createElement( "accountname" );
			QDomText accNameText = doc.createTextNode( ( me )
					? displayName
					: notKnown );
			accName.appendChild( accNameText );
			acc.appendChild( accName );

			QDomElement accStatus = doc.createElement( "accountstatus" );
			QDomText statusText = doc.createTextNode( ( me )
					? statusAsString( me->onlineStatus() )
					: notKnown ) ;
			accStatus.appendChild( statusText );

			// Dont add these if we're shutting down, because the result
			// would be quite weird.
			if ( !shuttingDown ) {

				// Add away message as an attribute, if one exists.
				if ( me->onlineStatus().status() == Kopete::OnlineStatus::Away &&
						!me->property("awayMessage").value().toString().isEmpty() ) {
					accStatus.setAttribute( "awayreason",
							me->property("awayMessage").value().toString() );
				}

				// Add the online status description as an attribute, if one exits.
				if ( !me->onlineStatus().description().isEmpty() ) {
					accStatus.setAttribute( "statusdescription",
							me->onlineStatus().description() );
				}
			}
			acc.appendChild( accStatus );

			if ( showAddresses )
			{
				QDomElement accAddress = doc.createElement( "accountaddress" );
				QDomText addressText = doc.createTextNode( ( me )
						? me->contactId()
						: notKnown );
				accAddress.appendChild( addressText );
				acc.appendChild( accAddress );
			}

			accounts.appendChild( acc );
		}
	}

	// write the XML to a temporary file
	KTempFile* file = new KTempFile();
	QTextStream *stream = file->textStream();
	stream->setEncoding( QTextStream::UnicodeUTF8 );
	doc.save( *stream, 4 );
	file->close();
	return file;
}

bool WebPresencePlugin::transform( KTempFile * src, KTempFile * dest )
{
#ifdef HAVE_XSLT
	bool retval = true;
	xmlSubstituteEntitiesDefault( 1 );
	xmlLoadExtDtdDefaultValue = 1;

	QFile sheet;

	switch ( resultFormatting ) {
	case WEB_XML:
		// Oops! We tried to call transform() but XML was requested.
		return false;
	case WEB_HTML:
		if ( useImagesInHTML ) {
			sheet.setName( locate( "appdata", "webpresence/webpresence_html_images.xsl" ) );
		} else {
			sheet.setName( locate( "appdata", "webpresence/webpresence_html.xsl" ) );
		}
		break;
	case WEB_XHTML:
		if ( useImagesInHTML ) {
			sheet.setName( locate( "appdata", "webpresence/webpresence_xhtml_images.xsl" ) );
		} else {
			sheet.setName( locate( "appdata", "webpresence/webpresence_xhtml.xsl" ) );
		}
		break;
	case WEB_CUSTOM:
		sheet.setName( userStyleSheet );
		break;
	default:
		// Shouldn't ever reach here.
		return false;
	}

	// TODO: auto / smart pointers would be useful here
	xsltStylesheetPtr cur = 0;
	xmlDocPtr doc = 0;
	xmlDocPtr res = 0;

	if ( !sheet.exists() ) {
		kdDebug(14309) << k_funcinfo << "ERROR: Style sheet not found" << endl;
		retval = false;
		goto end;
	}

	// is the cast safe?
	cur = xsltParseStylesheetFile( (const xmlChar *) sheet.name().latin1() );
	if ( !cur ) {
		kdDebug(14309) << k_funcinfo << "ERROR: Style sheet parsing failed" << endl;
		retval = false;
		goto end;
	}

	doc = xmlParseFile( QFile::encodeName( src->name() ) );
	if ( !doc ) {
		kdDebug(14309) << k_funcinfo << "ERROR: XML parsing failed" << endl;
		retval = false;
		goto end;
	}

	res = xsltApplyStylesheet( cur, doc, 0 );
	if ( !res ) {
		kdDebug(14309) << k_funcinfo << "ERROR: Style sheet apply failed" << endl;
		retval = false;
		goto end;
	}

	if ( xsltSaveResultToFile(dest->fstream(), res, cur) == -1 ) {
		kdDebug(14309) << k_funcinfo << "ERROR: Style sheet apply failed" << endl;
		retval = false;
		goto end;
	}

	// then it all worked!
	dest->close();

end:
	xsltCleanupGlobals();
	xmlCleanupParser();
	if (doc) xmlFreeDoc(doc);
	if (res) xmlFreeDoc(res);
	if (cur) xsltFreeStylesheet(cur);

	return retval;

#else
	Q_UNUSED( src );
	Q_UNUSED( dest );

	return false;
#endif
}

ProtocolList WebPresencePlugin::allProtocols()
{
	kdDebug( 14309 ) << k_funcinfo << endl;

	Kopete::PluginList plugins = Kopete::PluginManager::self()->loadedPlugins( "Protocols" );
	Kopete::PluginList::ConstIterator it;

	ProtocolList result;

	for ( it = plugins.begin(); it != plugins.end(); ++it ) {
		result.append( static_cast<Kopete::Protocol *>( *it ) );
	}

	return result;
}

QString WebPresencePlugin::statusAsString( const Kopete::OnlineStatus &newStatus )
{
	if (shuttingDown)
		return "OFFLINE";

	QString status;
	switch ( newStatus.status() )
	{
	case Kopete::OnlineStatus::Online:
		status = "ONLINE";
		break;
	case Kopete::OnlineStatus::Away:
		status = "AWAY";
		break;
	case Kopete::OnlineStatus::Offline:
	case Kopete::OnlineStatus::Invisible:
		status = "OFFLINE";
		break;
	default:
		status = "UNKNOWN";
	}

	return status;
}

void WebPresencePlugin::aboutToUnload()
{
	// Stop timer. Dont need it anymore.
	m_writeScheduler->stop();

	// Force statusAsString() report all accounts as OFFLINE.
	shuttingDown = true;

	// Do final update of webpresence file.
	slotWriteFile();

	emit readyForUnload();
}

// vim: set noet ts=4 sts=4 sw=4:
#include "webpresenceplugin.moc"
