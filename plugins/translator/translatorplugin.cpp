/*
    translatorplugin.cpp

    Kopete Translator plugin

    Copyright (c) 2001-2002 by Duncan Mac-Vicar Prett       <duncan@kde.org>
    Copyright (c) 2002-2003 by Olivier Goffart      <ogoffart@tiscalinet.be>

    Kopete    (c) 2002-2003 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include <qapplication.h>
#include <qregexp.h>
#include <qsignal.h>

#include <kdebug.h>
#include <kaction.h>
#include <kgenericfactory.h>

#include "kopetemetacontact.h"
#include "kopetecontactlist.h"
#include "kopetemessagemanagerfactory.h"
#include "kopeteview.h"

#include "translatorplugin.h"
#include "translatorprefs.h"
#include "translatordialog.h"
#include "translatorguiclient.h"

K_EXPORT_COMPONENT_FACTORY( kopete_translator, KGenericFactory<TranslatorPlugin> );

TranslatorPlugin::TranslatorPlugin( QObject *parent, const char *name,
	const QStringList &/*args*/ )
: KopetePlugin( parent, name )
{
	m_lc = 0; m_sc = 0;

	if ( pluginStatic_ )
		kdDebug(14308)<<"####"<<"Translator already initialized"<<endl;
	else
		pluginStatic_ = this;

	m_services.insert("babelfish", "BabelFish");
	m_services.insert("google", "Google");

	m_langs.insert("null", i18n("Unknown"));
	m_langs.insert("en", i18n("English"));
	m_langs.insert("zh", i18n("Chinese"));
	m_langs.insert("fr", i18n("French"));
	m_langs.insert("de", i18n("German"));
	m_langs.insert("it", i18n("Italian"));
	m_langs.insert("ja", i18n("Japanese"));
	m_langs.insert("ko", i18n("Korean"));
	m_langs.insert("pt", i18n("Portuguese"));
	m_langs.insert("ru", i18n("Russian"));
	m_langs.insert("es", i18n("Spanish"));

	/* English to .. */
	m_supported["babelfish"].append("en_zh");
	m_supported["babelfish"].append("en_fr");
	m_supported["babelfish"].append("en_de");
	m_supported["babelfish"].append("en_it");
	m_supported["babelfish"].append("en_ja");
	m_supported["babelfish"].append("en_ko");
	m_supported["babelfish"].append("en_pt");
	m_supported["babelfish"].append("en_es");
	/* Chinese to .. */
	m_supported["babelfish"].append("zh_en");
	/* French to ... */
	m_supported["babelfish"].append("fr_en");
	m_supported["babelfish"].append("fr_de");
	/* German to ... */
	m_supported["babelfish"].append("de_en");
	m_supported["babelfish"].append("de_fr");

	m_supported["babelfish"].append("it_en");
	m_supported["babelfish"].append("ja_en");
	m_supported["babelfish"].append("ko_en");
	m_supported["babelfish"].append("pt_en");
	m_supported["babelfish"].append("ru_en");
	m_supported["babelfish"].append("es_en");

	/* Google Service */
	m_supported["google"].append("en_de");
	m_supported["google"].append("en_es");
	m_supported["google"].append("en_fr");
	m_supported["google"].append("en_it");
	m_supported["google"].append("en_pt");
	m_supported["google"].append("de_en");
	m_supported["google"].append("es_en");
	m_supported["google"].append("fr_en");
	m_supported["google"].append("fr_de");
	m_supported["google"].append("it_en");
	m_supported["google"].append("pt_en");

	QMap<QString,QString>::ConstIterator i;

	for ( i = m_langs.begin(); i != m_langs.end() ; ++i )
	{
		m_langIntKeyMap[m_lc] = i.key();
		m_langKeyIntMap[i.key()] = m_lc;
		m_lc++;
	}

	for ( i = m_services.begin(); i != m_services.end() ; ++i )
	{
		m_servicesIntKeyMap[m_sc] = i.key();
		m_servicesKeyIntMap[i.key()] = m_sc;
		m_sc++;
	}

	m_prefs = new TranslatorPreferences ( "locale", this );

	connect( KopeteMessageManagerFactory::factory(), SIGNAL( aboutToDisplay( KopeteMessage & ) ), SLOT( slotIncomingMessage( KopeteMessage & ) ) );
	connect( KopeteMessageManagerFactory::factory(), SIGNAL( aboutToSend( KopeteMessage & ) ),    SLOT( slotOutgoingMessage( KopeteMessage & ) ) );

	connect( KopeteMessageManagerFactory::factory(), SIGNAL( messageManagerCreated( KopeteMessageManager * )) , SLOT( slotNewKMM( KopeteMessageManager * ) ) );

	QStringList keys;
	int k;
	for ( k=0; k <= m_lc; k++)
	{
		keys << m_langs[ languageKey(k) ];
	}
	m_actionLanguage=new KListAction(i18n("Set &Language"),"",0,  actionCollection() ,"contactLanguage");
	m_actionLanguage->setItems( keys );
	connect( m_actionLanguage, SIGNAL( activated() ), this, SLOT(slotSetLanguage()) );
	connect( KopeteContactList::contactList() , SIGNAL( metaContactSelected(bool) ) , this , SLOT(slotSelectionChanged(bool)));

	setXMLFile("translatorui.rc");

	//Add GUI action to all already existing kmm (if the plugin is launched when kopete already rining)
	QIntDict<KopeteMessageManager> sessions = KopeteMessageManagerFactory::factory()->sessions();
	QIntDictIterator<KopeteMessageManager> it( sessions );
	for ( ; it.current() ; ++it )
	{
		slotNewKMM(it.current());
	}
}

TranslatorPlugin::~TranslatorPlugin()
{
	pluginStatic_ = 0L;
}

TranslatorPlugin* TranslatorPlugin::plugin()
{
	return pluginStatic_ ;
}

TranslatorPlugin* TranslatorPlugin::pluginStatic_ = 0L;

void TranslatorPlugin::slotSelectionChanged(bool b)
{
	m_actionLanguage->setEnabled(b);

	if(!b)
		return;

	KopeteMetaContact *m=KopeteContactList::contactList()->selectedMetaContacts().first();

	if(!m)
		return;

	//Update the checkmark

	/*QStringList keys;
	int k;
	for ( k=0; k <= m_lc; k++)
	{
		keys << m_langs[ languageKey(k) ];
	}*/

	QString languageKey = m->pluginData( this, "languageKey" );
	if( !languageKey.isEmpty() && languageKey != "null" )
		m_actionLanguage->setCurrentItem( languageIndex( languageKey ) );
	else
		m_actionLanguage->setCurrentItem( languageIndex( "null" ) );
}

void TranslatorPlugin::slotNewKMM(KopeteMessageManager *KMM)
{
	new TranslatorGUIClient(KMM);
}

void TranslatorPlugin::slotIncomingMessage( KopeteMessage& msg )
{
	if(m_prefs->incommingMode()==DontTranslate)
		return;

//	kdDebug(14308) << "TranslatorPlugin::slotIncomingMessage " << m_prefs->incommingMode() << DontTranslate << endl;

	QString src_lang;
	QString dst_lang;

	if ( (msg.direction() == KopeteMessage::Inbound) && ( !msg.plainBody().isEmpty() ) )
	{
		KopeteMetaContact *from = msg.from()->metaContact();
		if( !from )
		{
			kdDebug(14308) << "TranslatorPlugin::slotIncomingMessage : No metaContact for source contact" << endl;
			return;
		}
		src_lang = from->pluginData( this, "languageKey" );
		if( src_lang.isEmpty() || src_lang == "null" )
		{
			kdDebug(14308) << "TranslatorPlugin::slotIncomingMessage : Cannot determine src Metacontact language (" << from->displayName() << ")" << endl;
			return;
		}

		dst_lang = m_prefs->myLang();

		if ( src_lang == dst_lang )
		{
			kdDebug(14308) << "TranslatorPlugin::slotIncomingMessage : Src and Dst languages are the same" << endl;
			return;
		}

		/* We search for src_dst */

		QStringList s = m_supported[ m_prefs->service() ];
		QStringList::ConstIterator i;

		for ( i = s.begin(); i != s.end() ; ++i )
		{
			if ( *i == src_lang + "_" + dst_lang )
			{
				sendTranslation(msg , translateMessage( msg.plainBody() , src_lang, dst_lang));
				return;
			}
		}
	}
	else
	{
		kdDebug(14308) << "TranslatorPlugin::slotIncomingMessage , outgoing or empty body" << endl;
	}
}

void TranslatorPlugin::slotOutgoingMessage( KopeteMessage& msg )
{
/*	kdDebug(14308) << "[Translator] Outgoing message " << msg.timestamp().toString("hhmmsszzz") << endl;
	kdDebug(14308) << "[Translator] Outgoing info: " << endl
		<< msg.body() << endl << "Direction: " << msg.direction();*/

	if(m_prefs->outgoingMode()==DontTranslate)
		return;


	QString src_lang;
	QString dst_lang;

	if ( ( msg.direction() == KopeteMessage::Outbound ) && ( !msg.plainBody().isEmpty() ) )
	{
		src_lang = m_prefs->myLang();
//		kdDebug(14308) << "[Translator] ( Outgoing ) My lang is: " << src_lang << endl;

		// Sad, we have to consider only the first To: metacontact
		KopeteMetaContact *to = msg.to().first()->metaContact();
		if( !to )
		{
			kdDebug(14308) << "TranslatorPlugin::slotOutgoingMessage : No metaContact for first contact" << endl;
			return;
		}
		dst_lang = to->pluginData( this, "languageKey" );
		if( dst_lang.isEmpty() || dst_lang == "null" )
		{
			kdDebug(14308) << "TranslatorPlugin::slotOutgoingMessage :  Cannot determine dst Metacontact language (" << to->displayName() << ")" << endl;
			return;
		}

		if ( src_lang == dst_lang )
		{
			kdDebug(14308) << "TranslatorPlugin::slotOutgoingMessage :  Src and Dst languages are the same" << endl;
			return;
		}

		/* We search for src_dst */

		QStringList s = m_supported[ m_prefs->service() ];
		QStringList::ConstIterator i;

		for ( i = s.begin(); i != s.end() ; ++i )
		{
			if ( *i == src_lang + "_" + dst_lang )
			{
				sendTranslation(msg , translateMessage( msg.plainBody() , src_lang, dst_lang));
				return;
//				kdDebug(14308) << "[Translator] Outgoing, DONE" << endl;
				return;
			}
			else
			{
//				kdDebug(14308) << "[Translator] ( Outgoing ) " << src_lang << " and " << dst_lang << " != " << *i<< endl;
			}
		}
	}
	else
	{
		kdDebug(14308) << "TranslatorPlugin::slotOutgoingMessage : incomming or empty body" << endl;
	}
}

void TranslatorPlugin::translateMessage(const QString &msg , const QString &from, const QString &to , QObject *obj , const char* slot)
{
	QSignal completeSignal;
	completeSignal.connect( obj, slot );

	QString result=translateMessage(msg,from,to);

	completeSignal.setValue( result );
	completeSignal.activate();




}


QString TranslatorPlugin::translateMessage(const QString &msg , const QString &from, const QString &to)
{
	if ( m_prefs->service() == "babelfish" )
		return babelTranslateMessage( msg ,from, to);
	if ( m_prefs->service() == "google" )
		return googleTranslateMessage( msg ,from, to);
	return QString::null;
}

QString TranslatorPlugin::googleTranslateMessage( const QString &msg , const QString &from, const QString &to)
{
	kdDebug(14308) << "[Translator] Google Translating: [" << from << "_" << to << "] " << endl
		<< msg << endl << endl;

	QString body, lp;
	KURL translatorURL;
	QCString postData;
	KIO::TransferJob *job;

	translatorURL = "http://translate.google.com/translate_t";

	//body = KURL::encode_string("*-*-* " + msg.body() + " *-*-*");
	body = KURL::encode_string( msg );

	lp = from + "|" + to;

	postData = "text=" + body +"&langpair=" + lp ;

	QString gurl = "http://translate.google.com/translate_t?text=" + body +"&langpair=" + lp;
	kdDebug(14308) << "[Translator] URL: " << gurl << endl;
	KURL geturl = gurl;

	//job = KIO::http_post( translatorURL, postData, true );
	job = KIO::get( geturl, false, true );

	//job->addMetaData("content-type", "application/x-www-form-urlencoded" );
	//job->addMetaData("referrer", "http://www.google.com");

	QObject::connect( job, SIGNAL(data( KIO::Job *,const QByteArray&)), this, SLOT(slotDataReceived( KIO::Job *,const QByteArray&)) );
	QObject::connect( job, SIGNAL(result( KIO::Job *)), this, SLOT(slotJobDone( KIO::Job *)) );

	/* KIO is async and we use a sync API, hay que dentrar a picarle nomas */
	while ( ! m_completed[ job ] )
		qApp->processEvents();

	QString data = QString::fromUtf8(m_data[job]);

	/* After hacks, we need to clean */
	m_data.remove( job );
	m_completed.remove( job );

	kdDebug(14308) << "[Translator]: Google response: "<< endl << data << endl;

	//QRegExp re("*-*-* (.*) *-*-*");
	QRegExp re("<textarea name=q rows=5 cols=45 wrap=PHYSICAL>(.*)</textarea>");
	re.setMinimal(true);
	re.match( data );

	QString translated = re.cap(1);

	return translated;

//	sendTranslation(msg,translated);
}

QString TranslatorPlugin::babelTranslateMessage( const QString &msg , const QString &from, const QString &to)
{
	kdDebug(14308) << "TranslatorPlugin::babelTranslateMessage : [" << from << "_" << to << "] " << endl ;

	QString body, lp;
	//KURL translatorURL;
	//QCString postData;
	KIO::TransferJob *job;

	//translatorURL = "http://babelfish.altavista.com/babelfish/tr";

	//body = KURL::encode_string("*-*-* " + msg.body() + " *-*-*");
	body = KURL::encode_string( msg);

	lp = from + "_" + to;

	//postData = "enc=utf8&doit=done&tt=urltext&urltext=" + body +"&lp=" + lp ;

	QString gurl = "http://babelfish.altavista.com/babelfish/tr?enc=utf8&doit=done&tt=urltext&urltext=" + body +"&lp=" + lp;
	kdDebug(14308) << "TranslatorPlugin::babelTranslateMessage : URL: " << gurl << endl;
	KURL geturl = gurl;

	//job = KIO::http_post( translatorURL, postData, true );
	job = KIO::get( geturl, false, true );

	//job->addMetaData("content-type", "application/x-www-form-urlencoded" );
	//job->addMetaData("referrer", "http://www.google.com");

	QObject::connect( job, SIGNAL(data( KIO::Job *,const QByteArray&)), this, SLOT(slotDataReceived( KIO::Job *,const QByteArray&)) );
	QObject::connect( job, SIGNAL(result( KIO::Job *)), this, SLOT(slotJobDone( KIO::Job *)) );

	/* KIO is async and we use a sync API, hay que dentrar a picarle nomas */
	while ( ! m_completed[ job ] )
		qApp->processEvents();

	QString data = QString::fromUtf8(m_data[job]);

	/* After hacks, we need to clean */
	m_data.remove( job );
	m_completed.remove( job );

//	kdDebug(14308) << "[Translator]: Babelfish response: "<< endl << data << endl;

	//QRegExp re("*-*-* (.*) *-*-*");
	QRegExp re("<Div style=padding:10px;>(.*)</div");
	re.setMinimal(true);
	re.match( data );

	QString translated = re.cap(1);
	return translated;

//	sendTranslation(msg,translated);
}

void TranslatorPlugin::sendTranslation(KopeteMessage &msg, const QString &translated)
{
	if ( translated.isEmpty() )
	{
		kdDebug(14308) << "TranslatorPlugin::sendTranslation - WARNING: Translated text is empty" <<endl;
		return;
	}

	TranslateMode mode=DontTranslate;

	switch (msg.direction())
	{
		case KopeteMessage::Outbound:
			mode=(TranslateMode)m_prefs->outgoingMode();
			break;
		case KopeteMessage::Inbound:
			mode=(TranslateMode)m_prefs->incommingMode();
			break;
		default:
			kdDebug(14308) << "TranslatorPlugin::sendTranslation - WARNING: can't determine if it is an incomming or outgoing message" <<endl;
	};

	switch (mode)
	{
		case JustTranslate:
			msg.setBody(translated, msg.format());
			break;
		case ShowOriginal:
			msg.setBody(i18n("%2\nAuto Translated: %1")
#if QT_VERSION < 0x030200
					.arg(translated).arg(msg.plainBody()), msg.format());
#else
					.arg(translated , msg.plainBody()), msg.format());
#endif
			break;
		case ShowDialog:
		{
			TranslatorDialog *d=new TranslatorDialog(translated);
			d->exec();
			msg.setBody(d->translatedText(),msg.format());
			delete d;
			break;
		}
		case DontTranslate:
		default:
			//do nothing
			break;
	};

}


void TranslatorPlugin::slotDataReceived ( KIO::Job *job, const QByteArray &data)
{
	m_data[job] += QCString( data, data.size()+1 );
}

void TranslatorPlugin::slotJobDone ( KIO::Job *job)
{
	m_completed[job] = true;
	QObject::disconnect( job, SIGNAL(data( KIO::Job *,const QByteArray&)), this, SLOT(slotDataReceived( KIO::Job *,const QByteArray&)) );
	QObject::disconnect( job, SIGNAL(result( KIO::Job *)), this, SLOT(slotJobDone( KIO::Job *)) );

}

void TranslatorPlugin::slotSetLanguage()
{
	KopeteMetaContact *m=KopeteContactList::contactList()->selectedMetaContacts().first();
	if( m && m_actionLanguage )
	{
		m->setPluginData( this, "languageKey", languageKey( m_actionLanguage->currentItem() ) );
	}
}

#include "translatorplugin.moc"

// vim: set noet ts=4 sts=4 sw=4:

