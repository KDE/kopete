/*
    Kopete Yahoo Protocol
    Handles logging into to the Yahoo service

    Copyright (c) 2004 Duncan Mac-Vicar P. <duncan@kde.org>

    Copyright (c) 2005 André Duffeck <duffeck@kde.org>

    Kopete (c) 2002-2005 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef LOGINTASK_H
#define LOGINTASK_H

#include "task.h"
#include "yahootypes.h"

class QString;
class YMSGTransfer;
class KJob;

namespace KIO
{
	class Job;
}

/**
@author Duncan Mac-Vicar
*/
class LoginTask : public Task
{
Q_OBJECT
public:
	LoginTask(Task *parent);
	~LoginTask();
	
	bool take(Transfer* transfer);
	virtual void onGo();

	void reset();
	void setStateOnConnect( Yahoo::Status status );
	void setVerificationWord( const QString &word );

	const QString &yCookie();
	const QString &cCookie();
	const QString &tCookie();
	const QString &loginCookie();
protected:
	virtual bool forMe( const Transfer* transfer ) const;
	enum State { InitialState, SentVerify, GotVerifyACK, SentAuth, GotAuthACK, SentAuthResp };
	void sendVerify();
	void sendAuth(YMSGTransfer* transfer);
	void sendAuthResp(YMSGTransfer* transfer);
	void sendAuthResp_0x0b(const QString &sn, const QString &seed, uint sessionID);
	void sendAuthResp_pre_0x0b(const QString &sn, const QString &seed);
	void handleAuthResp(YMSGTransfer *transfer);
	void parseCookies( YMSGTransfer *transfer );
	void sendAuthSixteenStage1(const QString& sn, const QString& seed);
	void sendAuthSixteenStage2(const QString& token);
	void sendAuthSixteenStage3(const QString& cryptString);
protected Q_SLOTS:
	void handleAuthSixteenStage1Data(KIO::Job*, const QByteArray& data);
	void handleAuthSixteenStage1Result(KJob*);
	void handleAuthSixteenStage2Data(KIO::Job*, const QByteArray& data);
	void handleAuthSixteenStage2Result(KJob*);

signals:
	void haveSessionID( uint );
	void haveCookies();
	void loginResponse( int, const QString& );
	void buddyListReady();
private:
	State mState;
	Yahoo::Status m_stateOnConnect;
	QString m_yCookie;
	QString m_tCookie;
	QString m_cCookie;
	QString m_loginCookie;
	QString m_verificationWord;
	QString m_stage1Data;
	QString m_stage2Data;
	QString m_challengeString;
	uint m_sessionID;
};

#endif
