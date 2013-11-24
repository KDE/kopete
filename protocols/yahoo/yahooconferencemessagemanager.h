/*
    yahooconferencemessagemanager.h - Yahoo Conference Message Manager

    Copyright (c) 2003 by Duncan Mac-Vicar Prett <duncan@kde.org>
    Copyright (c) 2005 by André Duffeck        <duffeck@kde.org>

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

#ifndef YAHOOCONFERENCEMESSAGEMANAGER_H
#define YAHOOCONFERENCEMESSAGEMANAGER_H

#include <kactioncollection.h>
#include "kopetechatsession.h"

class YahooContact;
class YahooAccount;

/**
 * @author Duncan Mac-Vicar Prett
 */
class YahooConferenceChatSession : public Kopete::ChatSession
{
	Q_OBJECT

public:
	YahooConferenceChatSession( const QString &m_yahooRoom, Kopete::Protocol *protocol, const Kopete::Contact *user, Kopete::ContactPtrList others );
	~YahooConferenceChatSession();

	void joined( YahooContact *c );
	void left( YahooContact *c );
	const QString &room();
	YahooAccount *account();
signals:
	void leavingConference( YahooConferenceChatSession *s );
protected slots:
	void slotMessageSent( Kopete::Message &message, Kopete::ChatSession * );
	void slotInviteOthers();
private:
	QString m_yahooRoom;

	KAction *m_actionInvite;
};

#endif

// vim: set noet ts=4 sts=4 tw=4:

