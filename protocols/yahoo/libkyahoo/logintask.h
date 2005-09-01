/*
    Kopete Yahoo Protocol
    Handles logging into to the Yahoo service

    Copyright (c) 2004 Duncan Mac-Vicar P. <duncan@kde.org>

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

#ifndef LOGINTASK_H
#define LOGINTASK_H

#include "task.h"

/**
@author Duncan Mac-Vicar
*/
class LoginTask : public Task
{
public:
	LoginTask(Task *parent);
	~LoginTask();
	
	bool take(Transfer* transfer);
	virtual void onGo();

protected:
	bool forMe( Transfer* transfer ) const;
	enum State { InitialState, SentVerify, GotVerifyACK, SentAuth, GotAuthACK, SentAuthResp };
	void sendVerify();
	void sendAuth(Transfer* transfer);
	void sendAuthResp(Transfer* transfer);
	void sendAuthResp_0x0b(const QString &sn, const QString &seed, uint sessionID);
	void sendAuthResp_pre_0x0b(const QString &sn, const QString &seed);
	State mState;
};

#endif
