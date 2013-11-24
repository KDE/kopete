/*
    translatorplugin.cpp

    Kopete Translator plugin

    Copyright (c) 2001-2002 by Duncan Mac-Vicar Prett <duncan@kde.org>
    Copyright (c) 2002-2004 by Olivier Goffart <ogoffart@kde.org>

    Kopete    (c) 2002-2004 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
    Patched by Francesco Rossi <redsh@email.it> in order to support new
    google translation page layout (13-sept-2007)
*/

#include "translatorplugin.h"

#include <qapplication.h>
#include <qregexp.h>
#include <qstring.h>
#include <qbytearray.h>
#include <qmetaobject.h>

#include <kdebug.h>
#include <kaction.h>
#include <kgenericfactory.h>
#include <kglobal.h>
#include <kconfig.h>
#include <kdeversion.h>
#include <kselectaction.h>
#include <kicon.h>
#include <kactioncollection.h>

#include "kopetemetacontact.h"
#include "kopetecontactlist.h"
#include "kopetechatsessionmanager.h"
#include "kopetecontact.h"

#include "translatordialog.h"
#include "translatorguiclient.h"
#include "translatorlanguages.h"
#include "translatorconfig.h"

K_PLUGIN_FACTORY( TranslatorPluginFactory, registerPlugin<TranslatorPlugin>(); )
K_EXPORT_PLUGIN( TranslatorPluginFactory( "kopete_translator" ) )

TranslatorPlugin::TranslatorPlugin( QObject *parent, const QVariantList & /* args */ )
: Kopete::Plugin( TranslatorPluginFactory::componentData(), parent )
{
	kDebug( 14308 ) ;


	if ( pluginStatic_ )
		kWarning( 14308 ) << "Translator already initialized";
	else
		pluginStatic_ = this;

	m_languages = new TranslatorLanguages;

	connect( Kopete::ChatSessionManager::self(), SIGNAL(aboutToDisplay(Kopete::Message&)),
		this, SLOT(slotIncomingMessage(Kopete::Message&)) );
	connect( Kopete::ChatSessionManager::self(), SIGNAL(aboutToSend(Kopete::Message&)),
		this, SLOT(slotOutgoingMessage(Kopete::Message&)) );
	connect( Kopete::ChatSessionManager::self(), SIGNAL(chatSessionCreated(Kopete::ChatSession*)),
		this, SLOT(slotNewKMM(Kopete::ChatSession*)) );

	m_actionLanguage = new KSelectAction( KIcon("preferences-desktop-locale"), i18n( "Set &Language" ), this );
	actionCollection()->addAction( "contactLanguage", m_actionLanguage );
	connect( m_actionLanguage, SIGNAL(triggered(int)), this, SLOT(slotSetLanguage()) );
	connect( Kopete::ContactList::self(), SIGNAL(metaContactSelected(bool)), this, SLOT(slotSelectionChanged(bool)) );

	setXMLFile( "translatorui.rc" );

	//Add GUI action to all already existing kmm (if the plugin is launched when kopete already running)
	const QList<Kopete::ChatSession*> sessions = Kopete::ChatSessionManager::self()->sessions();
	for (QList<Kopete::ChatSession*>::ConstIterator it= sessions.begin(); it!=sessions.end() ; ++it)
		slotNewKMM( *it );

	loadSettings();
	// When started, contactlist hasn't any selected metacontacts, but this action is available from contactlist
	// menubar, so we need to disable it.          
	// It is also important to do it after loadSettings because KSelectAction becomes enabled again after becoming non-empty
	m_actionLanguage->setEnabled( !Kopete::ContactList::self()->selectedMetaContacts().isEmpty() );
}

TranslatorPlugin::~TranslatorPlugin()
{
	kDebug( 14308 ) ;
	delete m_languages;
	pluginStatic_ = 0L;
}

TranslatorPlugin* TranslatorPlugin::plugin()
{
	return pluginStatic_;
}

TranslatorPlugin* TranslatorPlugin::pluginStatic_ = 0L;

void TranslatorPlugin::loadSettings()
{
	QString previousLang = m_languages->languageKey( m_service, m_actionLanguage->currentItem() );

	TranslatorConfig::self()->readConfig();
	int mode = 0;

	m_myLang = TranslatorConfig::defaultLanguage();
	m_service = TranslatorConfig::self()->service();

	if ( TranslatorConfig::incomingDontTranslate() )
		mode = DontTranslate;
	else if ( TranslatorConfig::incomingShowOriginal() )
		mode = ShowOriginal;
	else if ( TranslatorConfig::incomingTranslate() )
		mode = JustTranslate;

	m_incomingMode = mode;

	if ( TranslatorConfig::outgoingDontTranslate() )
		mode = DontTranslate;
	else if ( TranslatorConfig::outgoingShowOriginal() )
		mode = ShowOriginal;
	else if ( TranslatorConfig::outgoingTranslate() )
		mode = JustTranslate;
	else if ( TranslatorConfig::outgoingAsk() )
		mode = ShowDialog;

	m_outgoingMode = mode;

	// Setting menu entries

	QStringList keys;
	QMap<QString, QString> m = m_languages->languagesMap( m_service );
	for ( int k = 0; k <= m_languages->numLanguages( m_service ); k++ )
		keys << m[ m_languages->languageKey( m_service, k ) ];

	m_actionLanguage->setItems( keys );
	m_actionLanguage->setCurrentItem( m_languages->languageIndex( m_service, previousLang ) );
}

void TranslatorPlugin::slotSelectionChanged( bool b )
{
	m_actionLanguage->setEnabled( b );

	if ( !b )
		return;

	Kopete::MetaContact *m = Kopete::ContactList::self()->selectedMetaContacts().first();

	if( !m )
		return;
	QString languageKey = m->pluginData( this, "languageKey" );
	if ( !languageKey.isEmpty() && languageKey != "null" )
		m_actionLanguage->setCurrentItem( m_languages->languageIndex( m_service, languageKey ) );
	else
		m_actionLanguage->setCurrentItem( m_languages->languageIndex( m_service, "null" ) );
}

void TranslatorPlugin::slotNewKMM( Kopete::ChatSession *KMM )
{
	new TranslatorGUIClient( KMM );
}

void TranslatorPlugin::slotIncomingMessage( Kopete::Message &msg )
{
	if ( m_incomingMode == DontTranslate )
		return;

	QString src_lang;
	QString dst_lang;

	if ( ( msg.direction() == Kopete::Message::Inbound ) && !msg.plainBody().isEmpty() )
	{
		Kopete::MetaContact *from = msg.from()->metaContact();
		if ( !from )
		{
//			kDebug( 14308 ) << "No metaContact for source contact";
			return;
		}
		src_lang = from->pluginData( this, "languageKey" );
		if( src_lang.isEmpty() || src_lang == "null" )
		{
//			kDebug( 14308 ) << "Cannot determine src Metacontact language (" << from->displayName() << ")";
			return;
		}

		dst_lang = m_myLang;

		sendTranslation( msg, translateMessage( msg.plainBody(), src_lang, dst_lang ) );
	}
}

void TranslatorPlugin::slotOutgoingMessage( Kopete::Message &msg )
{
	if ( m_outgoingMode == DontTranslate )
		return;

	QString src_lang;
	QString dst_lang;

	if ( ( msg.direction() == Kopete::Message::Outbound ) && ( !msg.plainBody().isEmpty() ) )
	{
		src_lang = m_myLang;

		// Sad, we have to consider only the first To: metacontact
		Kopete::MetaContact *to = msg.to().first()->metaContact();
		if ( !to )
		{
//			kDebug( 14308 ) << "No metaContact for first contact";
			return;
		}
		dst_lang = to->pluginData( this, "languageKey" );
		if ( dst_lang.isEmpty() || dst_lang == "null" )
		{
//			kDebug( 14308 ) << "Cannot determine dst Metacontact language (" << to->displayName() << ")";
			return;
		}

		sendTranslation( msg, translateMessage( msg.plainBody(), src_lang, dst_lang ) );
	}
}

void TranslatorPlugin::translateMessage( const QString &msg, const QString &from, const QString &to, QObject *obj, const char* slot )
{
	QString result = translateMessage( msg, from, to );

	if(!result.isNull())
		QMetaObject::invokeMethod( obj, slot, Q_ARG( QString, result ) );
}

QString TranslatorPlugin::translateMessage( const QString &msg, const QString &from, const QString &to )
{
	if ( from == to )
	{
		kDebug( 14308 ) << "Src and Dst languages are the same";
		return QString();
	}

	// We search for src_dst
	if(! m_languages->supported( m_service ).contains( from + '_' + to ) )
	{
		kDebug( 14308 ) << from << '_' << to << " is not supported by service " << m_service;
		return QString();
	}


	if ( m_service == "babelfish" )
		return babelTranslateMessage( msg ,from, to );
	else if ( m_service == "google" )
		return googleTranslateMessage( msg ,from, to );
	else
		return QString();
}

QString TranslatorPlugin::googleTranslateMessage( const QString &msg, const QString &from, const QString &to )
{
	KUrl geturl ( QString("http://ajax.googleapis.com/ajax/services/language/translate?v=1.0&q=%1&langpair=%2|%3")
			.arg(QUrl::toPercentEncoding( msg ), from, to) );
	kDebug(14308) << "URL:" << geturl;

	KIO::TransferJob *job = KIO::get( geturl );
	//job = KIO::http_post( translatorURL, postData );

	//job->addMetaData( "content-type", "application/x-www-form-urlencoded" );
	//job->addMetaData( "referrer", "http://www.google.com" );

	QObject::connect( job, SIGNAL(data(KIO::Job*,QByteArray)), this, SLOT(slotDataReceived(KIO::Job*,QByteArray)) );
	QObject::connect( job, SIGNAL(result(KJob*)), this, SLOT(slotJobDone(KJob*)) );

	// KIO is async and we use a sync API, so use the processEvents hack to work around that
	// FIXME: We need to make the libkopete API async to get rid of this processEvents.
	//        It often causes crashes in the code. - Martijn
	while ( !m_completed[ job ] )
		qApp->processEvents();

	QString data = QString::fromUtf8( m_data[ job ] );

	// After hacks, we need to clean
	m_data.remove( job );
	m_completed.remove( job );

//	kDebug( 14308 ) << "Google response:"<< endl << data;

	QRegExp re( "\"translatedText\":\"(.*)\"" );

	re.setMinimal( true );
	re.indexIn( data );

	return re.cap( 1 );
}

QString TranslatorPlugin::babelTranslateMessage( const QString &msg, const QString &from, const QString &to )
{
	KUrl geturl ( "http://babelfish.yahoo.com/translate_txt" );

	QString body = QUrl::toPercentEncoding( msg );
	body.replace("%20", "+");

	QByteArray postData = QString( "ei=UTF-8&doit=done&fr=bf-res&intl=1&tt=urltext&trtext=%1&lp=%2_%3&btnTrTxt=Translate")
				.arg( body, from, to )
				.toLocal8Bit();
	kDebug(14308) << "URL:" << geturl << "(post data" << postData << ")";

	KIO::TransferJob *job = KIO::http_post( geturl, postData );
	job->addMetaData( "content-type", "Content-Type: application/x-www-form-urlencoded" );
	job->addMetaData( "referrer", "http://babelfish.yahoo.com/translate_txt" );

	QObject::connect( job, SIGNAL(data(KIO::Job*,QByteArray)), this, SLOT(slotDataReceived(KIO::Job*,QByteArray)) );
	QObject::connect( job, SIGNAL(result(KJob*)), this, SLOT(slotJobDone(KJob*)) );

	// KIO is async and we use a sync API, so use the processEvents hack to work around that
	// FIXME: We need to make the libkopete API async to get rid of this processEvents.
	//        It often causes crashes in the code. - Martijn
	while ( !m_completed[ job ] )
		qApp->processEvents();

	QString data = QString::fromUtf8( m_data[ job ] );

	// After hacks, we need to clean
	m_data.remove( job );
	m_completed.remove( job );

//	kDebug( 14308 ) << "Babelfish response: " << endl << data;

	QRegExp re( "<div style=\"padding:0.6em;\">(.*)</div>" );
	re.setMinimal( true );
	re.indexIn( data );

	return re.cap( 1 );
}

void TranslatorPlugin::sendTranslation( Kopete::Message &msg, const QString &translated )
{
	if ( translated.isEmpty() )
	{
		kWarning( 14308 ) << "Translated text is empty";
		return;
	}

	TranslateMode mode = DontTranslate;

	switch ( msg.direction() )
	{
	case Kopete::Message::Outbound:
		mode = TranslateMode( m_outgoingMode );
		break;
	case Kopete::Message::Inbound:
		mode = TranslateMode( m_incomingMode );
		break;
	default:
		kWarning( 14308 ) << "Can't determine if it is an incoming or outgoing message";
	};

	switch ( mode )
	{
	case JustTranslate:
		if ( msg.format() & Qt::PlainText )
			msg.setPlainBody( translated );
		else
			msg.setHtmlBody ( translated );
		break;
	case ShowOriginal:
		if ( msg.format() & Qt::PlainText )
			msg.setPlainBody( i18n( "%2 \nAuto Translated: \n%1", translated, msg.plainBody() ) );
		else
			msg.setHtmlBody( i18n( "%2 <br/>Auto Translated: <br/>%1", translated, msg.plainBody() ) );
		break;
	case ShowDialog:
	{
		TranslatorDialog *d = new TranslatorDialog( translated );
		d->exec();
		if ( msg.format() & Qt::PlainText )
			msg.setPlainBody (d->translatedText() );
		else
			msg.setHtmlBody( d->translatedText() );
		delete d;
		break;
	}
	case DontTranslate:
	default:
		//do nothing
		break;
	};
}

void TranslatorPlugin::slotDataReceived ( KIO::Job *job, const QByteArray &data )
{
	m_data[ job ].append(data);
}

void TranslatorPlugin::slotJobDone ( KJob *job )
{
	KIO::Job *kioJob = static_cast<KIO::Job*>(job);
	m_completed[ kioJob ] = true;
	QObject::disconnect( kioJob, SIGNAL(data(KIO::Job*,QByteArray)), this, SLOT(slotDataReceived(KIO::Job*,QByteArray)) );
	QObject::disconnect( kioJob, SIGNAL(result(KJob*)), this, SLOT(slotJobDone(KJob*)) );
}

void TranslatorPlugin::slotSetLanguage()
{
	Kopete::MetaContact *m = Kopete::ContactList::self()->selectedMetaContacts().first();
	if( m && m_actionLanguage )
		m->setPluginData( this, "languageKey", m_languages->languageKey( m_service, m_actionLanguage->currentItem() ) );
}

#include "translatorplugin.moc"

// vim: set noet ts=4 sts=4 sw=4:

