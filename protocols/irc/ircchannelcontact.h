/***************************************************************************
                          ircchannelcontact.h  -  description
                             -------------------
    begin                : Thu Feb 20 2003
    copyright            : (C) 2003 by nbetcher
    email                : nbetcher@kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef IRCCHANNELCONTACT_H
#define IRCCHANNELCONTACT_H

#include "kopetecontact.h"
#include "kirc.h"

#include <qptrlist.h>

class IRCIdentity;
class KopeteMetaContact;
class KActionCollection;
class KAction;
class KopeteMessageManager;
class KopeteMessage;

class IRCChanPrivUser : public KopeteContact
{
public:
	/* This class provides a KopeteContact for each user on the channel.
	I decided not to make these all IRCContacts since that would be a waste
	of memory and completely unneeded. */
	IRCChanPrivUser(IRCIdentity *, const QString &nickname, KIRC::UserClass);

	// Nickname stuff
	void setNickname(const QString &nickname) { mNickname = nickname; }
	const QString &nickname() { return mNickname; }

	// Userclass stuff
	void setUserclass(KIRC::UserClass userclass) { mUserclass = userclass; }
	KIRC::UserClass userclass() { return mUserclass; }

	// KopeteContact stuff
	QString statusIcon() const;
private:
	KIRC::UserClass mUserclass;
	QString mNickname;
};

class IRCChannelContact : public KopeteContact
{
Q_OBJECT
public:
	IRCChannelContact(IRCIdentity *, const QString &channel, KopeteMetaContact *metac);

	~IRCChannelContact();

	// START: Virtual reimplmentations from KopeteContact, see kopetecontact.h:
	bool isReachable();
	KActionCollection *customContextMenuActions();
	KopeteMessageManager* manager( bool canCreate = false );
	QString statusIcon() const;
	// FINISH
	
private slots:
	void slotConnectedToServer();
	void slotUserJoinedChannel(const QString &, const QString &);
	void slotJoin();
	void slotPart();
	void slotMessageManagerDestroyed();
	void slotSendMsg(KopeteMessage &message, KopeteMessageManager *);
	void slotUserPartedChannel(const QString &user, const QString &channel, const QString &reason);
	void slotConnectionClosed();
	void slotNewMessage(const QString &originating, const QString &target, const QString &message);
	void slotNamesList(const QString &channel, const QString &nickname, const int);
private:
	IRCIdentity *mIdentity;
	QString mChannelName;
	KopeteMetaContact *mMetaContact;
	KIRC *mEngine;
	QPtrList<KopeteContact> mContact;
	QPtrList<KopeteContact> mMyself;
	KopeteMessageManager *mMsgManager;
	QMap<QString, IRCChanPrivUser * > mMembers;

	// KAction stuff:
	KActionCollection *mCustomActions;
	KAction *actionJoin;
	KAction *actionPart;
};

#endif
