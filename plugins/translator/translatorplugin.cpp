/*
    translatorplugin.cpp

    Kopete Translator plugin

    Copyright (c) 2001-2002 by Duncan Mac-Vicar Prett   <duncan@kde.org>

    Kopete    (c) 2002 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "translatorplugin.h"
#include "translatorplugin.moc"

#include <qcolor.h>
#include <qcstring.h>
#include <qstring.h>
#include <qregexp.h>
#include <qmap.h>

#include <kdebug.h>
#include <kaction.h>
#include <klocale.h>
#include <kgenericfactory.h>
#include <kio/global.h>
#include <kio/job.h>
#include <kstandarddirs.h>
#include <kurl.h>

#include "kopete.h"
#include "kopetecontact.h"
#include "kopetemessage.h"
#include "kopetemetacontact.h"
#include "translatorprefs.h"

K_EXPORT_COMPONENT_FACTORY( kopete_translator, KGenericFactory<TranslatorPlugin> );

TranslatorPlugin::TranslatorPlugin( QObject *parent, const char *name,
	const QStringList &/*args*/ )
: KopetePlugin( parent, name )
{
	m_actionCollection=0L;
	m_actionLanguage=0L;

	m_lc = 0; m_sc = 0;

	if ( pluginStatic_ )
		kdDebug()<<"####"<<"Translator already initialized"<<endl;
	else
		pluginStatic_ = this;

	m_services.insert("babelfish", "BabelFish");

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
 
	connect( kopeteapp, SIGNAL(aboutToDisplay(KopeteMessage&)),
		 SLOT(slotIncomingMessage(KopeteMessage&)) );
	connect( kopeteapp, SIGNAL(aboutToSend(KopeteMessage&)),
		 SLOT(slotOutgoingMessage(KopeteMessage&)) );
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

void TranslatorPlugin::init()
{
}

bool TranslatorPlugin::unload()
{
	return true;
}

bool TranslatorPlugin::serialize( KopeteMetaContact *metaContact,
			  QStringList &strList  ) const
{
	if ( m_langMap.contains( metaContact ) )
		strList<<  m_langMap[ metaContact ] ;
	else
		strList<< "null";
	return true;
}

void TranslatorPlugin::deserialize( KopeteMetaContact *metaContact, const QStringList& data )
{
	m_langMap[ metaContact ] = data.first();
	kdDebug() << "[Translator] Deserialize " << metaContact->displayName() << " lang is " << data.first() << endl;

}

KActionCollection *TranslatorPlugin::customContextMenuActions(KopeteMetaContact *m)
{
    QStringList keys;

	QMap<QString,QString>::ConstIterator it;
	int k;

	for ( k=0; k <= m_lc; k++)
	{
		keys << m_langs[ languageKey(k) ];
	}

	if(m_actionLanguage)
		delete m_actionLanguage;

	if( m_actionCollection )
		delete m_actionCollection;

	m_actionCollection = new KActionCollection(this);
	m_actionLanguage=new KListAction(i18n("Set &Language"),"",0,  m_actionCollection ,"m_actionLanguage");

	m_actionLanguage->setItems( keys );

	if ( ( m_langMap[m] != 0L ) || ( m_langMap[m] != "null") )
		m_actionLanguage->setCurrentItem( languageIndex(m_langMap[m]) );

	connect( m_actionLanguage, SIGNAL( activated() ), this, SLOT(slotSetLanguage()) );
	m_actionCollection->insert(m_actionLanguage);
	m_currentMetaContact=m;
	return m_actionCollection;
}

void TranslatorPlugin::slotIncomingMessage( KopeteMessage& msg )
{
    kdDebug() << "[Translator] Incoming message " << msg.timestamp().toString("hhmmsszzz") << endl;

	QString src_lang;
    QString dst_lang;

	if ( (msg.direction() == KopeteMessage::Inbound) && ( msg.body() != QString::null ) )
    {
        KopeteMetaContact *from = msg.from()->metaContact();
		if ( m_langMap.contains( from ) && (m_langMap[from] != "null"))
    	{
			src_lang = m_langMap[ from ];
		}
		else
		{
            kdDebug() << "[Translator] ( Incoming) Cannot determine src Metacontact language (" << from->displayName() << ")" << endl;
			return;
		}

		dst_lang = m_prefs->myLang();

		if ( src_lang == dst_lang )
		{
            kdDebug() << "[Translator] ( Incoming) Src and Dst languages are the same" << endl;
			return;
		}

		/* We search for src_dst */

		QStringList s = m_supported[ m_prefs->service() ];
		QStringList::ConstIterator i;
        
		for ( i = s.begin(); i != s.end() ; ++i )
		{
			if ( *i == src_lang + "_" + dst_lang )
			{
				translateMessage( msg , src_lang, dst_lang);
				return;
			}
		}				
	}
	else
	{
		kdDebug() << "[Translator] Incoming, but empty body" << endl;
	}	
}

void TranslatorPlugin::slotOutgoingMessage( KopeteMessage& msg )
{
    kdDebug() << "[Translator] Outgoing message " << msg.timestamp().toString("hhmmsszzz") << endl;
    kdDebug() << "[Translator] Outgoing info: " << endl
				<< msg.body() << endl << "Direction: " << msg.direction();
    QString src_lang;
    QString dst_lang;

	if ( ( msg.direction() == KopeteMessage::Outbound ) && ( msg.body() != QString::null ) )
    {
		src_lang = m_prefs->myLang();
		kdDebug() << "[Translator] ( Outgoing ) My lang is: " << src_lang << endl;

		/* Sad, we have to consideer only the first To: metacontact only */
		KopeteMetaContact *to = msg.to().first()->metaContact();
		if ( m_langMap.contains( to ) && (m_langMap[to] != "null"))
    	{
			dst_lang = m_langMap[ to ];
            kdDebug() << "[Translator] ( Outgoing ) remote lang is: " << dst_lang << endl;
		}
		else
		{
            kdDebug() << "[Translator] ( Outgoing ) Cannot determine dst Metacontact language (" << to->displayName() << ")" << endl;
			return;
		}

		if ( src_lang == dst_lang )
		{
            kdDebug() << "[Translator] ( Outgoing ) Src and Dst languages are the same" << endl;
			return;
		}

		/* We search for src_dst */

		QStringList s = m_supported[ m_prefs->service() ];
		QStringList::ConstIterator i;

		for ( i = s.begin(); i != s.end() ; ++i )
		{
			if ( *i == src_lang + "_" + dst_lang )
			{
				translateMessage( msg , src_lang, dst_lang);
				kdDebug() << "[Translator] Outgoing, DONE" << endl;
				return;
			}
			else
			{
				kdDebug() << "[Translator] ( Outgoing ) " << src_lang << " and " << dst_lang << " != " << *i<< endl;
			}
		}				
	}
	else
	{
		kdDebug() << "[Translator] Outgoing, but empty body" << endl;
	}
}

void TranslatorPlugin::translateMessage( KopeteMessage &msg , const QString &from, const QString &to)
{
	kdDebug() << "[Translator] Translating: [" << from << "_" << to << "] " << endl
			<< msg.body() << endl << endl;
	
	QString body, lp;
	KURL translatorURL;
	QCString postData;
	KIO::TransferJob *job;

	translatorURL = "http://babel.altavista.com/tr";

	//body = KURL::encode_string("*-*-* " + msg.body() + " *-*-*");
    body = KURL::encode_string( msg.body() );

	lp = from + "_" + to;

	postData = "enc=utf8&doit=done&tt=urltext&urltext=" + body +"&lp=" + lp ;

	QString gurl = "http://babel.altavista.com/tr?enc=utf8&doit=done&tt=urltext&urltext=" + body +"&lp=" + lp;
	kdDebug() << "[Translator] URL: " << gurl << endl;
	KURL geturl = gurl;

	//job = KIO::http_post( translatorURL, postData, true );
	job = KIO::get( geturl, false, true );
	
	//job->addMetaData("content-type", "application/x-www-form-urlencoded" );
	//job->addMetaData("referrer", "http://www.google.com");

	QObject::connect( job, SIGNAL(data( KIO::Job *,const QByteArray&)), this, SLOT(slotDataReceived( KIO::Job *,const QByteArray&)) );
	QObject::connect( job, SIGNAL(result( KIO::Job *)), this, SLOT(slotJobDone( KIO::Job *)) );

	/* KIO is async and we use a sync API, hay que dentrar a picarle nomas */
	while ( ! m_completed[ job ] )
		kopeteapp->processEvents();

	QString data = QString::fromUtf8(m_data[job]);

	/* After hacks, we need to clean */
	m_data.remove( job );
	m_completed.remove( job );

	kdDebug() << "[Translator]: Babelfish response: "<< endl << data << endl;

	//QRegExp re("*-*-* (.*) *-*-*");
    QRegExp re("<Div style=padding:10px;>(.*)</div");
	re.setMinimal(true);
	re.match( data );

	QString translated = re.cap(1);

	if ( translated != QString::null )
		msg.setBody(translated);
	else
		msg.setBody(msg.body());
}

void TranslatorPlugin::slotDataReceived ( KIO::Job *job, const QByteArray &data)
{
	m_data[job] = m_data[job] + data;
}

void TranslatorPlugin::slotJobDone ( KIO::Job *job)
{
	m_completed[job] = true;
	QObject::disconnect( job, SIGNAL(data( KIO::Job *,const QByteArray&)), this, SLOT(slotDataReceived( KIO::Job *,const QByteArray&)) );
	QObject::disconnect( job, SIGNAL(result( KIO::Job *)), this, SLOT(slotJobDone( KIO::Job *)) );

}

void TranslatorPlugin::slotSetLanguage()
{
	if( m_actionLanguage && m_currentMetaContact)
	{
		m_langMap[ m_currentMetaContact ]= languageKey( m_actionLanguage->currentItem() );
	}
}
