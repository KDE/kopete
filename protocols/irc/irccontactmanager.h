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

#include <qdict.h>
#include <qobject.h>
#include <qstring.h>
#include <qstringlist.h>

class IRCContact;
class IRCAccount;

class IRCServerContact;
class IRCChannelContact;
class IRCUserContact;

namespace KIRC
{
class Engine;
}

namespace Kopete
{
class Contact;
class MetaContact;
}

class KopeteView;

class QTimer;

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
		IRCContactManager(const QString &nickName, IRCAccount *account, const char *name=0);

		IRCAccount *account() const { return m_account; }

		IRCServerContact *myServer() const { return m_myServer; }
		IRCUserContact *mySelf() const { return m_mySelf; }

		IRCChannelContact *findChannel(const QString &channel, Kopete::MetaContact *m=0);
		IRCChannelContact *existChannel(const QString &channel) const;

		IRCUserContact *findUser(const QString &nick, Kopete::MetaContact *m=0);
		IRCUserContact *existUser(const QString &nick) const;

		IRCContact *findContact(const QString &nick, Kopete::MetaContact *m=0);
		IRCContact *existContact( const QString &id ) const;

		QValueList<IRCChannelContact*> findChannelsByMember( IRCUserContact *contact );

		static IRCContact *existContact(const KIRC::Engine *engine, const QString &nick);

	public slots:
		void unregister(Kopete::Contact *contact);
		void unregisterUser(Kopete::Contact *contact, bool force = false );
		void unregisterChannel(Kopete::Contact *contact, bool force = false );

		void addToNotifyList(const QString &nick);
		void removeFromNotifyList(const QString &nick);
		void checkOnlineNotifyList();

	signals:
		void privateMessage(IRCContact *from, IRCContact *to, const QString &message);

	private slots:
		void slotNewMessage(const QString &originating, const QString &channel, const QString &message);
		void slotNewPrivMessage(const QString &originating, const QString &, const QString &message);
		void slotIsonRecieved();
		void slotIsonTimeout();
		void slotNewNickChange(const QString &oldnick, const QString &newnick);
		void slotContactAdded( Kopete::MetaContact *contact );

	private:
		QDict<IRCChannelContact> m_channels;
		QDict<IRCUserContact> m_users;

		IRCAccount *m_account;
		IRCServerContact *m_myServer;
		IRCUserContact *m_mySelf;

		QStringList m_NotifyList;
		QTimer *m_NotifyTimer;
		bool isonRecieved;
		int socketTimeout;

		static const QRegExp isChannel;
};

#endif

