/*
    msnsecureloginhandler.cpp - SSL login for MSN protocol

    Copyright (c) 2005      by Michaël Larouche       <michael.larouche@kdemail.net>

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
//	kdDebug(14140) << k_funcinfo << endl;
}

void MSNSecureLoginHandler::login()
{
	// Retrive the login server.
	// Do a reload and don't show the progress.
	KIO::Job *getLoginServer = KIO::get(KURL("https://nexus.passport.com/rdr/pprdr.asp"), true, false);

	getLoginServer->addMetaData("cookies", "manual");
	getLoginServer->addMetaData("cache", "reload");
	getLoginServer->addMetaData("PropagateHttpHeader", "true");

	connect(getLoginServer, SIGNAL(result(KIO::Job *)), this, SLOT(slotLoginServerReceived(KIO::Job* )));
}

void MSNSecureLoginHandler::slotLoginServerReceived(KIO::Job *loginJob)
{
	if(!loginJob->error())
	{
		// Retrive the HTTP header
		QString httpHeaders = loginJob->queryMetaData("HTTP-Headers");

		// Get the login URL using QRegExp
		QRegExp rx("PassportURLs: DARealm=(.*),DALogin=(.*),DAReg=");
		rx.search(httpHeaders);

		// Set the loginUrl and loginServer
		QString loginUrl = rx.cap(2);
		QString loginServer = loginUrl.section('/', 0, 0);

		kdDebug(14140) << k_funcinfo << loginServer << endl;

		QString authURL = "https://" + loginUrl;

		KIO::Job *authJob = KIO::get(KURL(authURL), true, false);
		authJob->addMetaData("cookies", "manual");

		QString authRequest = "Authorization: Passport1.4 "
								"OrgVerb=GET,"
								"OrgURL=http%3A%2F%2Fmessenger%2Emsn%2Ecom,"
								"sign-in=" + KURL::encode_string(m_accountId) +
								",pwd=" + KURL::encode_string( m_password ).replace(',',"%2C") +
								"," + m_authentification + "\r\n";

//   warning, this debug contains the password
//		kdDebug(14140) << k_funcinfo << "Auth request: " << authRequest << endl;

		authJob->addMetaData("customHTTPHeader", authRequest);
		authJob->addMetaData("SendLanguageSettings", "false");
		authJob->addMetaData("PropagateHttpHeader", "true");
		authJob->addMetaData("cookies", "manual");
		authJob->addMetaData("cache", "reload");
		
		connect(authJob, SIGNAL(result(KIO::Job *)), this, SLOT(slotTweenerReceived(KIO::Job* )));
	}
	else
	{
		kdDebug(14140) << k_funcinfo << loginJob->errorString() << endl;

		emit loginFailed();
	}	
}

void MSNSecureLoginHandler::slotTweenerReceived(KIO::Job *authJob)
{
	if(!authJob->error())
	{
		QString httpHeaders = authJob->queryMetaData("HTTP-Headers");

// 		kdDebug(14140) << k_funcinfo << "HTTP headers: " << httpHeaders << endl;

		// Check if we get "401 Unauthorized", thats means it's a bad password.
		if(httpHeaders.contains(QString::fromUtf8("401 Unauthorized")))
		{
// 			kdDebug(14140) << k_funcinfo << "MSN Login Bad password." << endl;
			emit loginBadPassword();
		}
		else
		{
			QRegExp rx("from-PP='(.*)'");
			rx.search(httpHeaders);
			QString ticket = rx.cap(1);
		
	//		kdDebug(14140) << k_funcinfo << "Received ticket: " << ticket << endl;
	
			emit loginSuccesful(ticket);
		}
	}
	else
	{
		kdDebug(14140) << k_funcinfo << authJob->errorString() << endl;

		emit loginFailed();
	}
}
#include "msnsecureloginhandler.moc"
