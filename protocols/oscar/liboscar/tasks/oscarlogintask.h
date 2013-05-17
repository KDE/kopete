/*
    Kopete Oscar Protocol
    oscarlogintask.h - Handles logging into to the OSCAR service

    Copyright (c) 2004 Matt Rogers <mattr@kde.org>
    Copyright (c) 2007 Roman Jarosz <kedgedev@centrum.cz>

    Kopete (c) 2002-2007 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef OSCARLOGINTASK_H
#define OSCARLOGINTASK_H

#include "task.h"

using namespace Oscar;

class OscarLoginTask : public Task
{
Q_OBJECT
public:
	OscarLoginTask( Task* parent );
	~OscarLoginTask();
	bool take( Transfer* transfer );
	virtual void onGo();

	//Protocol specific stuff
	const QByteArray& cookie() const;
	const QString& bosHost() const;
	const QString& bosPort() const;
	bool bosEncrypted() const;
	const QString& bosSSLName() const;

protected:
	bool forMe( const Transfer* transfer ) const;

signals:
	void haveAuthKey();

private:
	//! Encodes a password using MD5
	QByteArray encodePassword() const;

	//! Send SNAC 0x17, 0x06
	void sendAuthStringRequest();

	//! Handle SNAC 0x17, 0x07
	void processAuthStringReply();

	//! Handle SNAC 0x17, 0x03
	void handleLoginResponse();

	//! Parse the error codes to generate a reason why sign-on failed
	//Massive code duplication with CloseConnectionTask
	bool parseDisconnectCode( int error, QString& reason );

private slots:
	//! Send SNAC 0x17, 0x02
	void sendLoginRequest();

private:
	//! The authorization key to use when encoding the password
	QByteArray m_authKey;

	//! The all important connection cookie
	QByteArray m_cookie;

	//! The new BOS Host
	QString m_bosHost;

	//! The new BOS Port
	QString m_bosPort;

	bool m_bosEncrypted;

	QString m_bosSSLName;

};

#endif
