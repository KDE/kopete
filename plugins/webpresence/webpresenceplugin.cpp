/*
   webpresenceplugin.cpp

   Kopete Web Presence plugin

   Copyright     2005-2006 by Tommi Rantala   <tommi.rantala@cs.helsinki.fi>
   Copyright (c) 2002,2003 by Will Stephenson <will@stevello.free-online.co.uk>

   Kopete    (c) 2002-2006 by the Kopete developers  <kopete-devel@kde.org>

 *************************************************************************
 *                                                                    	 *
 * This program is free software; you can redistribute it and/or modify  *
 * it under the terms of the GNU General Public License as published by  *
 * the Free Software Foundation; either version 2 of the License, or     *
 * (at your option) any later version.                                   *
 *                                                                       *
 *************************************************************************
 */

#include "webpresenceplugin.h"

#include <QtCore/QTimer>
#include <QtCore/QFile>
#include <QtCore/QTextCodec>
#include <QtCore/QTextStream>
#include <QtCore/QList>
#include <QtCore/QDateTime>
#include <QtXml/QDomDocument>

#include <kdebug.h>
#include <kgenericfactory.h>
#include <kio/job.h>
#include <kmessagebox.h>
#include <ktemporaryfile.h>
#include <kstandarddirs.h>

#include <libxml/parser.h>
#include <libxml/tree.h>

#include <libxslt/xsltconfig.h>
#include <libxslt/xsltInternals.h>
#include <libxslt/transform.h>
#include <libxslt/xsltutils.h>

#include "kopetepluginmanager.h"
#include "kopeteprotocol.h"
#include "kopeteaccountmanager.h"
#include "kopeteaccount.h"
#include "kopetecontact.h"

#include "webpresenceconfig.h"

K_PLUGIN_FACTORY(WebPresencePluginFactory, registerPlugin<WebPresencePlugin>();)
K_EXPORT_PLUGIN(WebPresencePluginFactory( "kopete_webpresence" ))


WebPresencePlugin::WebPresencePlugin( QObject *parent, const QVariantList& /*args*/ )
	: Kopete::Plugin( WebPresencePluginFactory::componentData(), parent ),
	shuttingDown( false ), resultFormatting( WEB_HTML ), m_output(0)
{
	m_writeScheduler = new QTimer( this );
	connect ( m_writeScheduler, SIGNAL(timeout()), this, SLOT(slotWriteFile()) );
	connect( Kopete::AccountManager::self(), SIGNAL(accountRegistered(Kopete::Account*)),
				this, SLOT(listenToAllAccounts()) );
	connect( Kopete::AccountManager::self(), SIGNAL(accountUnregistered(const Kopete::Account*)),
				this, SLOT(listenToAllAccounts()) );

	connect(this, SIGNAL(settingsChanged()), this, SLOT(slotSettingsChanged()) );
	slotSettingsChanged();
	listenToAllAccounts();
}

WebPresencePlugin::~WebPresencePlugin()
{
        delete m_output;
}

void WebPresencePlugin::slotSettingsChanged()
{
	// Force reading config
	WebPresenceConfig::self()->readConfig();
	
	resultFormatting = WEB_UNDEFINED;

	if ( WebPresenceConfig::self()->formatHTML() ) {
		resultFormatting = WEB_HTML;
	} else if ( WebPresenceConfig::self()->formatXHTML() ) {
		resultFormatting = WEB_XHTML;
	} else if ( WebPresenceConfig::self()->formatXML() ) {
		resultFormatting = WEB_XML;
	} else if ( WebPresenceConfig::self()->formatStylesheet() ) {
		resultFormatting = WEB_CUSTOM;
		userStyleSheet = WebPresenceConfig::self()->formatStylesheetURL();
	}

	// Default to HTML, if we don't get anything useful from config file.
	if ( resultFormatting == WEB_UNDEFINED )
		resultFormatting = WEB_HTML;

	// Update file
	slotWriteFile();
}

void WebPresencePlugin::listenToAllAccounts()
{
	// Connect to signals notifying of all accounts' status changes.
	ProtocolList protocols = allProtocols();

	for ( ProtocolList::Iterator it = protocols.begin();
			it != protocols.end(); ++it )
	{
		QList<Kopete::Account*> accounts = Kopete::AccountManager::self()->accounts( *it );
		foreach(Kopete::Account *account, accounts)
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
						SLOT(slotWaitMoreStatusChanges()) ) ;
		QObject::connect( account->myself(),
						SIGNAL(onlineStatusChanged( Kopete::Contact *,
								const Kopete::OnlineStatus &,
								const Kopete::OnlineStatus & ) ),
						this,
						SLOT(slotWaitMoreStatusChanges()) );
	}
}

void WebPresencePlugin::slotWaitMoreStatusChanges()
{
	if ( !m_writeScheduler->isActive() )
		m_writeScheduler->start( WebPresenceConfig::self()->uploadFrequency() * 1000 );
}

void WebPresencePlugin::slotWriteFile()
{
	m_writeScheduler->stop();

	// generate the (temporary) XML file representing the current contact list
	const KUrl dest = WebPresenceConfig::self()->uploadURL();
	if ( dest.isEmpty() || !dest.isValid() )
	{
		kDebug(14309) << "url is empty or not valid. NOT UPDATING!";
		return;
	}

	KTemporaryFile* xml = generateFile();
	xml->setAutoRemove( true );

	switch( resultFormatting ) {
	case WEB_XML:
		m_output = xml;
		xml = 0L;
		break;
	case WEB_HTML:
	case WEB_XHTML:
	case WEB_CUSTOM:
		m_output = new KTemporaryFile();
		m_output->open();

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
	KUrl src( m_output->fileName() );
	KIO::FileCopyJob *job = KIO::file_move( src, dest, -1, KIO::Overwrite | KIO::HideProgressInfo );
	connect( job, SIGNAL(result(KJob*)),
			SLOT(slotUploadJobResult(KJob*)) );
}

void WebPresencePlugin::slotUploadJobResult( KJob *job )
{
	if ( job->error() ) {
		kDebug(14309) << "Error uploading presence info.";
		KMessageBox::queuedDetailedError( 0, i18n("An error occurred when uploading your presence page.\nCheck the path and write permissions of the destination."), 0, displayName() );
		delete m_output;
		m_output = 0L;
	}
}

KTemporaryFile* WebPresencePlugin::generateFile()
{
	// generate the (temporary) XML file representing the current contact list
	kDebug( 14309 );
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
	const QString userName = WebPresenceConfig::self()->showThisName();
	if ( !WebPresenceConfig::self()->showName() && !userName.isEmpty() )
		nameText = doc.createTextNode( userName );
	else
		nameText = doc.createTextNode( notKnown );
	name.appendChild( nameText );
	root.appendChild( name );

	// insert the list of the user's accounts
	QDomElement accounts = doc.createElement( "accounts" );
	root.appendChild( accounts );

	QList<Kopete::Account*> list = Kopete::AccountManager::self()->accounts();
	// If no accounts, stop here
	if ( !list.isEmpty() )
	{
		foreach(Kopete::Account *account, list)
		{
			QDomElement acc = doc.createElement( "account" );
			//output += h.openTag( "account" );

			QDomElement protoName = doc.createElement( "protocol" );
			QDomText protoNameText = doc.createTextNode(
					account->protocol()->pluginId() );
			protoName.appendChild( protoNameText );
			acc.appendChild( protoName );

			Kopete::Contact* me = account->myself();
			QString displayName = me->displayName();
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

			// Do not add these if we're shutting down, because the result
			// would be quite weird.
			if ( !shuttingDown ) {

				// Add away message as an attribute, if one exists.
				if ( (me->onlineStatus().status() == Kopete::OnlineStatus::Away ||
						me->onlineStatus().status() == Kopete::OnlineStatus::Busy) &&
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

			if ( WebPresenceConfig::self()->includeIMAddress() )
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
	KTemporaryFile* file = new KTemporaryFile();
	file->setAutoRemove(false);
	file->open();
	QTextStream stream( file );
	stream.setCodec(QTextCodec::codecForName("UTF-8"));
	doc.save( stream, 4 );
	stream.flush();
	return file;
}

bool WebPresencePlugin::transform( KTemporaryFile * src, KTemporaryFile * dest )
{
	bool retval = true;
	xmlSubstituteEntitiesDefault( 1 );
	xmlLoadExtDtdDefaultValue = 1;

	QFile sheet;

	switch ( resultFormatting ) {
	case WEB_XML:
		// Oops! We tried to call transform() but XML was requested.
		return false;
	case WEB_HTML:
		if ( WebPresenceConfig::self()->useImagesHTML() ) {
			sheet.setFileName( KStandardDirs::locate( "appdata", "webpresence/webpresence_html_images.xsl" ) );
		} else {
			sheet.setFileName( KStandardDirs::locate( "appdata", "webpresence/webpresence_html.xsl" ) );
		}
		break;
	case WEB_XHTML:
		if ( WebPresenceConfig::self()->useImagesHTML() ) {
			sheet.setFileName( KStandardDirs::locate( "appdata", "webpresence/webpresence_xhtml_images.xsl" ) );
		} else {
			sheet.setFileName( KStandardDirs::locate( "appdata", "webpresence/webpresence_xhtml.xsl" ) );
		}
		break;
	case WEB_CUSTOM:
		sheet.setFileName( userStyleSheet.path() );
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
		kDebug(14309) << "ERROR: Style sheet not found";
		retval = false;
		goto end;
	}

	// is the cast safe?
	cur = xsltParseStylesheetFile( (const xmlChar *) sheet.fileName().toLatin1().data() );
	if ( !cur ) {
		kDebug(14309) << "ERROR: Style sheet parsing failed";
		retval = false;
		goto end;
	}

	doc = xmlParseFile( QFile::encodeName( src->fileName() ) );
	if ( !doc ) {
		kDebug(14309) << "ERROR: XML parsing failed";
		retval = false;
		goto end;
	}

	res = xsltApplyStylesheet( cur, doc, 0 );
	if ( !res ) {
		kDebug(14309) << "ERROR: Style sheet apply failed";
		retval = false;
		goto end;
	}


	if ( xsltSaveResultToFd(dest->handle(), res, cur) == -1 ) {
		kDebug(14309) << "ERROR: Style sheet apply failed";
		retval = false;
		goto end;
	}

	// then it all worked!

end:
	xsltCleanupGlobals();
	xmlCleanupParser();
	if (doc) xmlFreeDoc(doc);
	if (res) xmlFreeDoc(res);
	if (cur) xsltFreeStylesheet(cur);

	return retval;
}

ProtocolList WebPresencePlugin::allProtocols()
{
	kDebug( 14309 ) ;

	Kopete::PluginList plugins = Kopete::PluginManager::self()->loadedPlugins( "Protocols" );
	Kopete::PluginList::ConstIterator it;

	ProtocolList result;

	for ( it = plugins.constBegin(); it != plugins.constEnd(); ++it ) {
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
	case Kopete::OnlineStatus::Busy:
		status = "BUSY";
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
	// Stop timer. Do not need it anymore.
	m_writeScheduler->stop();

	// Force statusAsString() report all accounts as OFFLINE.
	shuttingDown = true;

	// Do final update of webpresence file.
	slotWriteFile();

	emit readyForUnload();
}

// vim: set noet ts=4 sts=4 sw=4:
#include "webpresenceplugin.moc"
