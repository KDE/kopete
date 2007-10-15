/*
    Kopete Yahoo Protocol
    Handles conferences

    Copyright (c) 2005 André Duffeck <duffeck@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef CONFERENCETASK_H
#define CONFERENCETASK_H

#include "task.h"

class YMSGTransfer;

/**
@author André Duffeck
*/
class ConferenceTask : public Task
{
	Q_OBJECT
public:
	ConferenceTask(Task *parent);
	~ConferenceTask();
	
	bool take(Transfer *transfer);
	bool forMe( const Transfer* transfer ) const;

	void joinConference( const QString &room, const QStringList &members );
	void declineConference( const QString &room, const QStringList &members, const QString &msg );
	void leaveConference( const QString &room, const QStringList &members );
	void sendMessage( const QString &room, const QStringList &members, const QString &msg );
	void inviteConference( const QString &room, const QStringList &members, const QString &msg );
	void addInvite( const QString &room, const QStringList &who, const QStringList &members, const QString &msg );
signals:
	void gotInvite( const QString &who, const QString &room, const QString &msg, const QStringList &members);
	void gotMessage( const QString &who, const QString &room, const QString &msg );
	void userJoined( const QString &who, const QString &room );
	void userLeft( const QString &who, const QString &room );
	void userDeclined( const QString &who, const QString &room, const QString &msg );
private:
	void parseInvitation( YMSGTransfer *transfer );
	void parseMessage( YMSGTransfer *transfer );
	void parseUserJoined( YMSGTransfer *transfer );
	void parseUserLeft( YMSGTransfer *transfer );
	void parseUserDeclined( YMSGTransfer *transfer );
};

#endif
