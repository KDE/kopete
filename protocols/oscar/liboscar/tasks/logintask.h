/*
    Kopete Oscar Protocol
    logintask.h - Handles logging into to the AIM or ICQ service

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

#ifndef _OSCAR_LOGINTASK_H_
#define _OSCAR_LOGINTASK_H_

#include "oscartypes.h"
#include "task.h"

class OscarLoginTask;
class CloseConnectionTask;

using namespace Oscar;

class QString;
class Transfer;


/**
 * \short Handle OSCAR login - stage 1
 *
 * OSCAR login is divided into two stages. The first stage handles the connection negotiation
 * so that we can get a BOS server to connect to and start stage two of the login process
 * This class handles the first stage of the OSCAR login process
 * For more info about the OSCAR login process, visit http://iserverd1.khstu.ru/oscar
 */
class StageOneLoginTask : public Task
{
Q_OBJECT
public:
	StageOneLoginTask( Task* parent );
	~StageOneLoginTask();
	bool take( Transfer* transfer );

	//Protocol specific stuff
	
	//! Get the BOS cookie
	const QByteArray& loginCookie() const;
	
	//! Get the BOS server
	const QString& bosServer() const;

	//! Get the BOS port
	const QString& bosPort() const;

	bool bosEncrypted() const;

	const QString& bosSSLName() const;

	//! Get the error code, if there is one
	int errorCode() const;

	//! Get the error reason so it can be displayed
	const QString& errorReason() const;


public slots:
	void closeTaskFinished();
	void loginTaskFinished();

protected:
	bool forMe( const Transfer* transfer ) const;

private:
	
	//Tasks we want to control
	OscarLoginTask* m_loginTask;
	CloseConnectionTask* m_closeTask;

	//Private data we get from the tasks
	QByteArray m_cookie;
	QString m_bosServer;
	QString m_bosPort;
	bool m_bosEncrypted;
	QString m_bosSSLName;

};

/**
 * \short Handle OSCAR Login - stage 2
 *
 * Oscar login is divided into two stages. The first stage handles the connection negotiation
 * so that we can get a BOS server to connect to for the second stage. This class handles the
 * second stage of Oscar login that establishes various things like rate limits, contact lists,
 * and SNAC family versions
 */

class ServerVersionsTask;
class RateInfoTask;

class StageTwoLoginTask : public Task
{
Q_OBJECT
public:
	StageTwoLoginTask( Task* parent );
	~StageTwoLoginTask();
	bool take( Transfer* transfer );
	void onGo();

	//protocol specifics
	//! Set the cookie to send to the server
	void setCookie( const QByteArray& newCookie );
	
	//! Get the cookie to send to the server
	const QByteArray& cookie();

	QString host() const;
	QString port() const;

public slots:

	//! Start the rate info task
	void versionTaskFinished();
	
	//! The rate info task is finished. Start the other ones
	void rateTaskFinished();
	
protected:
	bool forMe( const Transfer* transfer ) const;

private:
	QByteArray m_cookie;
	QString m_host, m_port;

	//tasks
	ServerVersionsTask* m_versionTask;
	RateInfoTask* m_rateTask;
};

#endif

//kate: tab-width 4; indent-mode csands;
