/*
    ircaccount.h - IRC Account

    Copyright (c) 2002      by Nick Betcher <nbetcher@kde.org>

    Kopete    (c) 2002      by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef IRCACCOUNT_H
#define IRCACCOUNT_H

#include "kopeteaccount.h"

#include <qstring.h>

class IRCProtocol;
class IRCChannelContact;
class IRCUserContact;
class KopeteMetaContact;
class KopeteMessage;
class KopeteContact;
class KopeteMessageManager;
class KIRC;
class KAction;
class KActionMenu;

class IRCAccount : public KopeteAccount
{
	Q_OBJECT

public:
	IRCAccount(const QString &accountid, const IRCProtocol *p);
	~IRCAccount();

	// Returns the KopeteContact of the user
	IRCUserContact *mySelf() { return mMySelf; }

	// Returns the KIRC engine instance
	KIRC *engine() { return mEngine; }

	// Returns the IRCProtocol instance for contacts
	const IRCProtocol *protocol() { return mProtocol; }

	/**
	 * Attempts to find a IRCChannelContact with the specified name in
	 * our registered channels list. If one exists, it is returned. If not, a new contact
	 * is created for it, and either assigned to the passed in @ref KopeteMestaContact
	 * if it exists, or assigned to a temporary @ref KopeteMetaContact. If a temporary
	 * @ref KopeteMetaContact is created, it is *not* added to the contact list by this
	 * function.
	 */
	IRCChannelContact *findChannel(const QString &name, KopeteMetaContact *m = 0L);

	/**
	 * Attempts to find a IRCUserContact with the specified name in
	 * our registered users list. If one exists, it is returned. If not, a new contact
	 * is created for it, and either assigned to the passed in @ref KopeteMestaContact
	 * if it exists, or assigned to a temporary @ref KopeteMetaContact. If a temporary
	 * @ref KopeteMetaContact is created, it is *not* added to the contact list by this
	 * function.
	 */
	IRCUserContact *findUser(const QString &name, KopeteMetaContact *m = 0L);

	/**
	 * Unregisters a channel contact. This function checks the channels conversation
	 * count (the number of conversations it is taking part in), and if it is 0, it deletes the
	 * contact and removes it from the registered channels list. Channels in the contact
	 * list are never deleted.
	 */
	void unregisterChannel(const QString &name);

	/**
	 * Unregisters a user contact. This function checks the users conversation
	 * count (the number of conversations it is taking part in), and if it is 0, it deletes the
	 * contact and removes it from the registered channels list. Users in the contact
	 * list are never deleted.
	 */
	void unregisterUser(const QString &name);

	bool addContact( const QString &contact, const QString &displayName, KopeteMetaContact *m);

	virtual KActionMenu *actionMenu();

	virtual KopeteContact *myself() const { return (KopeteContact*)mMySelf; }

	virtual void setAway(bool);

	virtual bool isConnected();

public slots:
	void successfullyChangedNick(const QString &, const QString &);
	virtual void connect();
	virtual void disconnect();

private slots:
	void slotConnectedToServer();
	void slotConnectionClosed();
	void slotNewPrivMessage(const QString &originating, const QString &target, const QString &message);

private:
	const IRCProtocol *mProtocol;
	KopeteMessageManager *mManager;
	QString mNickName;
	QString mServer;
	uint mPort;
	IRCUserContact *mMySelf;
	KIRC *mEngine;
	QMap<QString, IRCChannelContact * > mChannels;
	QMap<QString, IRCUserContact * > mUsers;
	KAction *actionOnline;
	KAction *actionOffline;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

