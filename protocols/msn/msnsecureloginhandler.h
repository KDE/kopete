/*
    msnsecureloginhandler.h - SSL login for MSN protocol

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
#ifndef MSNSECURELOGINHANDLER_H
#define MSNSECURELOGINHANDLER_H

#include <qobject.h>

namespace KIO
{
	class Job;
	class MetaData;
}

/**
 * This class handle the login process. It connect to the .NET Password service and retrive the ticket(tweener) to login.
 * Use KIO.
 *
 * @author Michaël Larouche <michael.larouche@kdemail.net>
*/
class MSNSecureLoginHandler : public QObject
{
Q_OBJECT
public:
    MSNSecureLoginHandler(const QString &accountId, const QString &password, const QString &authParameters);

    ~MSNSecureLoginHandler();

	void login();

signals:
	/**
	 * TODO: return to const QString &
	 */
	void loginSuccesful(QString ticket);
	void loginBadPassword();
	void loginFailed();

private slots:
	void slotLoginServerReceived(KIO::Job *);
	/**
	 * We have received our ticket to login.
	 */
	void slotTweenerReceived(KIO::Job *);

private:
	/**
	 * Store the password.
	 */
	QString m_password;
	/**
	 * Store the accountId.
	 */
	QString m_accountId;
	/**
	 * Store the authentification parameters
	 */
	QString m_authentification;

	void displayMetaData(KIO::MetaData data);
};

#endif
