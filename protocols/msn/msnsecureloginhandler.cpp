/*
    msnsecureloginhandler.cpp - SSL login for MSN protocol

    Copyright (c) 2005      by Michaël Larouche       <larouche@kde.org>

    Kopete    (c) 2002-2005 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/
#include "msnsecureloginhandler.h"

// Qt includes
#include <qregexp.h>

// KDE includes
#include <kio/job.h>
#include <kurl.h>
#include <kdebug.h>

MSNSecureLoginHandler::MSNSecureLoginHandler(const QString &accountId, const QString &password, const QString &authParameters)
  : m_password(password), m_accountId(accountId), m_authentification(authParameters)
{
	
}

MSNSecureLoginHandler::~MSNSecureLoginHandler()
{
//	kDebug(14140) ;
}

void MSNSecureLoginHandler::login()
{
	// Retrive the login server.
	// Do a reload and don't show the progress.
	KIO::Job *getLoginServer = KIO::get(KUrl("https://nexus.passport.com/rdr/pprdr.asp"), KIO::Reload, KIO::HideProgressInfo);

	getLoginServer->addMetaData("cookies", "manual");
	getLoginServer->addMetaData("cache", "reload");
	getLoginServer->addMetaData("PropagateHttpHeader", "true");

	connect(getLoginServer, SIGNAL(result(KJob *)), this, SLOT(slotLoginServerReceived(KJob* )));
}

void MSNSecureLoginHandler::slotLoginServerReceived(KJob *job)
{
	KIO::Job *loginJob = static_cast<KIO::Job*>(job);
	if(!loginJob->error())
	{
		// Retrive the HTTP header
		QString httpHeaders = loginJob->queryMetaData("HTTP-Headers");

		// Get the login URL using QRegExp
		QRegExp rx("PassportURLs: DARealm=(.*),DALogin=(.*),DAReg=");
		rx.setCaseSensitivity(Qt::CaseInsensitive);
		rx.indexIn(httpHeaders);

		// Set the loginUrl and loginServer
		QString loginUrl = rx.cap(2);
		QString loginServer = loginUrl.section('/', 0, 0);

		kDebug(14140) << loginServer;

		QString authURL = "https://" + loginUrl;

		KIO::Job *authJob = KIO::get(KUrl(authURL), KIO::Reload, KIO::HideProgressInfo);
		authJob->addMetaData("cookies", "manual");

		QString authRequest = "Authorization: Passport1.4 "
								"OrgVerb=GET,"
								"OrgURL=http%3A%2F%2Fmessenger%2Emsn%2Ecom,"
								"sign-in=" + QUrl::toPercentEncoding(m_accountId) +
								",pwd=" + QUrl::toPercentEncoding( m_password ).replace(',',"%2C") +
								',' + m_authentification + "\r\n";

//   warning, this debug contains the password
//		kDebug(14140) << "Auth request: " << authRequest;

		authJob->addMetaData("customHTTPHeader", authRequest);
		authJob->addMetaData("SendLanguageSettings", "false");
		authJob->addMetaData("PropagateHttpHeader", "true");
		authJob->addMetaData("cookies", "manual");
		authJob->addMetaData("cache", "reload");
		
		connect(authJob, SIGNAL(result(KJob *)), this, SLOT(slotTweenerReceived(KJob* )));
	}
	else
	{
		kDebug(14140) << loginJob->errorString();

		emit loginFailed();
	}	
}

void MSNSecureLoginHandler::slotTweenerReceived(KJob *job)
{
	KIO::Job *authJob = static_cast<KIO::Job*>(job);
	if(!authJob->error())
	{
		QString httpHeaders = authJob->queryMetaData("HTTP-Headers");

// 		kDebug(14140) << "HTTP headers: " << httpHeaders;

		// Check if we get "401 Unauthorized", thats means it's a bad password.
		if(httpHeaders.contains(QString::fromUtf8("401 Unauthorized")))
		{
// 			kDebug(14140) << "MSN Login Bad password.";
			emit loginBadPassword();
		}
		else
		{
			QRegExp rx("from-PP='(.*)'");
			rx.indexIn(httpHeaders);
			QString ticket = rx.cap(1);
		
	//		kDebug(14140) << "Received ticket: " << ticket;
	
			emit loginSuccesful(ticket);
		}
	}
	else
	{
		kDebug(14140) << authJob->errorString();

		emit loginFailed();
	}
}
#include "msnsecureloginhandler.moc"
