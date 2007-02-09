/*
 Kopete Oscar Protocol
 userinfotask.h - Handle sending and receiving info requests for users

 Copyright (c) 2004-2005 Matt Rogers <mattr@kde.org>

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
#ifndef USERINFOTASK_H
#define USERINFOTASK_H

#include "task.h"
#include <QMap>
#include <qstring.h>
#include "userdetails.h"

class Transfer;

/**
Handles user information requests that are done via SNAC 02,05 and 02,06

@author Kopete Developers
*/
class UserInfoTask : public Task
{
Q_OBJECT
public:
	UserInfoTask( Task* parent );
	~UserInfoTask();

	enum { Profile = 0x0001, General = 0x0002, AwayMessage = 0x0003, Capabilities = 0x0004 };

	//! Task implementation
	bool forMe( const Transfer* transfer ) const;
	bool take( Transfer* transfer );
	void onGo();

	void requestInfoFor( const QString& userId, unsigned int types );
	UserDetails getInfoFor( quint16 sequence ) const;
	QString contactForSequence( quint16 sequence ) const;


signals:
	void gotInfo( quint16 seqNumber );
	void receivedProfile( const QString& contact, const QString& profile );
	void receivedAwayMessage( const QString& contact, const QString& message );

private:
	QMap<quint16, UserDetails> m_sequenceInfoMap;
	QMap<quint16, QString> m_contactSequenceMap;
	QMap<quint16, unsigned int> m_typesSequenceMap;
	quint16 m_seq;

};



#endif

//kate: indent-mode csands; tab-width 4;
