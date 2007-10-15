/*
    yahoochatchatsession.h - Yahoo Chat Chatsession

    Copyright (c) 2003 by Duncan Mac-Vicar Prett <duncan@kde.org>
    Copyright (c) 2006 by André Duffeck        <duffeck@kde.org>

    Kopete    (c) 2002-2006 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef YAHOOCHATCHATSESSION_H
#define YAHOOCHATCHATSESSION_H

#include "kopetechatsession.h"

class YahooContact;
class YahooAccount;

/**
 * @author André Duffeck
 */
class YahooChatChatSession : public Kopete::ChatSession
{
	Q_OBJECT
public:
	YahooChatChatSession( Kopete::Protocol *protocol, const Kopete::Contact *user, Kopete::ContactPtrList others );
	~YahooChatChatSession();

	void joined( YahooContact *c, bool suppressNotification = false );
	void left( YahooContact *c );
	YahooAccount *account();

	void setTopic( const QString & topic );
	void setHandle( const QString &handle ) { m_handle = handle; }
	QString handle() { return m_handle; }

	void removeAllContacts();
signals:
	void leavingChat( YahooChatChatSession *s );
protected slots:
	void slotMessageSent( Kopete::Message &message, Kopete::ChatSession * );
private:
	QString m_topic;
	QString m_handle;
};

#endif

// vim: set noet ts=4 sts=4 tw=4:

