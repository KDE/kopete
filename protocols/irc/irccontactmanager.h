/*
    irccontactmanager.h - Manager of IRC Contacts

    Copyright (c) 2003      by Michel Hermier <michel.hermier@wanadoo.fr>

    Kopete    (c) 2003      by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef IRCCONTACTMANAGER_H
#define IRCCONTACTMANAGER_H

#include <qmap.h>
#include <qobject.h>
#include <qstring.h>

#include "ircaccount.h"

class QTimer;

class KopeteMetaContact;
class KopeteView;

class KIRC;
class IRCServerContact;
class IRCChannelContact;
class IRCUserContact;
class IRCContact;

/**
 * @author Michel Hermier <michel.hermier@wanadoo.fr>
 *
 * This class is the repository for all the reference of the @ref IRCContact childs.
 * It manage the life cycle of all the @ref IRCServerContact, @ref IRCChannelContact and @ref IRCUserContact objects for the given account.
 */
class IRCContactManager
	: public QObject
{
	Q_OBJECT

	public:
		IRCContactManager(const QString &nickName, const QString &serverName, IRCAccount *account, const char *name=0);
		
		IRCAccount *account() const { return m_account; }
		IRCServerContact *myServer() const { return m_myServer; }
		IRCUserContact *mySelf() const { return m_mySelf; }
		KIRC *engine() const { return m_engine; }
		
		IRCServerContact *findServer(const QString &server, KopeteMetaContact *m=0);
		bool existServer(const QString &server);
	
		IRCChannelContact *findChannel(const QString &channel, KopeteMetaContact *m=0);
		bool existChannel(const QString &channel);
		
		IRCUserContact *findUser(const QString &nick, KopeteMetaContact *m=0);
		bool existUser(const QString &nick);
		
	public slots:
		void unregister(KopeteContact *contact);
		
		void unregisterUser(const QString &nick);
		void unregisterUser(KopeteContact *contact, bool force = false);
		
		void addToNotifyList(const QString &nick);
		void removeFromNotifyList(const QString &nick);
		
		void unregisterChannel(const QString &channel);
		void unregisterChannel(KopeteContact *contact, bool force = false);
		
		void unregisterServer(const QString &server);
		void unregisterServer(KopeteContact *contact);
		
		void checkOnlineNotifyList();
	
	signals:
		void privateMessage(IRCContact *from, IRCContact *to, const QString &message);
		void action(IRCContact *from, IRCContact *to, const QString &action);
	
	protected slots:
		//KIRC slots
		void slotNewMessage(const QString &originating, const QString &channel, const QString &message);
		void slotNewPrivMessage(const QString &originating, const QString &, const QString &message);
		void slotNewAction(const QString &originating, const QString &channel, const QString &action);
		void slotNewPrivAction(const QString &originating, const QString &, const QString &action);
	
	protected:
		IRCAccount *m_account;
		KIRC *m_engine;
	
		QMap<QString, IRCServerContact *> m_servers;
		QMap<QString, IRCChannelContact *> m_channels;
		QMap<QString, IRCUserContact *> m_users;
	
		IRCServerContact *m_myServer;
		IRCUserContact *m_mySelf;
	
		QStringList m_NotifyList;
		QTimer *m_NotifyTimer;
	
	private slots:
		void slotNewNickChange(const QString &oldnick, const QString &newnick);

};




#endif // IRCCONTACTMANAGER_H

