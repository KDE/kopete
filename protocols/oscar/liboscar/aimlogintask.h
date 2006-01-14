/*
    Kopete Oscar Protocol
    aimlogintask.h - Handles logging into to the AIM service

    Copyright (c) 2004 Matt Rogers <mattr@kde.org>

    Kopete (c) 2002-2004 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef _OSCAR_AIMLOGINTASK_H_
#define _OSCAR_AIMLOGINTASK_H_

#include "task.h"

using namespace Oscar;

class AimLoginTask : public Task
{
Q_OBJECT
public:
	AimLoginTask( Task* parent );
	~AimLoginTask();
	bool take( Transfer* transfer );
	virtual void onGo();
	
	//Protocol specific stuff
	const QByteArray& cookie() const;
	const QString& bosHost() const;
	const QString& bosPort() const;

protected:
	bool forMe( Transfer* transfer ) const;

signals:
	void haveAuthKey();
	
private:
	//! Encodes a password using MD5
	void encodePassword( QByteArray& digest ) const;

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
	
};

#endif
