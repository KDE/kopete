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

#include <kdebug.h>
#include <kgenericfactory.h>
#include <kstandarddirs.h>
#include <kurl.h>
#include <kio/global.h>
#include <kio/job.h>
#include <qcstring.h>
#include <qregexp.h>


#include "kopete.h"
#include "kopetemessage.h"
#include "kopetecontact.h"
#include "kopetemetacontact.h"

#include "translatorplugin.h"
#include "translatorprefs.h"

#include <qregexp.h>
#include <qcolor.h>

K_EXPORT_COMPONENT_FACTORY( kopete_translator, KGenericFactory<TranslatorPlugin> );

TranslatorPlugin::TranslatorPlugin( QObject *parent, const char *name,
			      const QStringList &/*args*/ )
	: Plugin( parent, name )
{

	m_prefs = new TranslatorPreferences ( "locale", this );


	connect( kopeteapp, SIGNAL(aboutToDisplay(KopeteMessage&)),
		 SLOT(slotIncomingMessage(KopeteMessage&)) );
	connect( kopeteapp, SIGNAL(aboutToSend(KopeteMessage&)),
		 SLOT(slotOutgoingMessage(KopeteMessage&)) );


}

TranslatorPlugin::~TranslatorPlugin()
{
}

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
}

void TranslatorPlugin::slotIncomingMessage( KopeteMessage& msg )
{
    kdDebug() << "[Translator] Incoming message " << msg.timestamp().toString("hhmmsszzz") << endl;
    if ( msg.direction() == KopeteMessage::Inbound )
		translateMessage( msg , "en", "es");
}

void TranslatorPlugin::slotOutgoingMessage( KopeteMessage& msg )
{
    kdDebug() << "[Translator] Outgoing message " << msg.timestamp().toString("hhmmsszzz") << endl;
    if ( msg.direction() == KopeteMessage::Outbound )
		translateMessage( msg , "es", "en");
}

void TranslatorPlugin::translateMessage( KopeteMessage& msg , const QString &from, const QString &to)
{
    kdDebug() << "[Translator] Translating " << from << " " << to << " " << msg.timestamp().toString("hhmmsszzz") << endl;
	
	QString body, lp;
	KURL translatorURL;
	QCString postData;
	KIO::TransferJob *job;

	translatorURL = "http://babel.altavista.com/tr";

	body = KURL::encode_string("x-x-x " + msg.body() + " x-x-x");
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

	QString data = m_data[job];

	/* After hacks, we need to clean */
	m_data.remove( job );
	m_completed.remove( job );

	kdDebug() << "[Translator]: Babelfish response: "<< endl << data << endl;

	QRegExp re("x-x-x (.*) x-x-x");
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



