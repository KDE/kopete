/***************************************************************************
                          irccontact.h  -  description
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

#ifndef IRCCONTACT_H
#define IRCCONTACT_H

#include <qptrlist.h>
#include <qmap.h>

#include "kirc.h"
#include "kopetecontact.h"

class KopeteMessageManager;
class KopeteMetaContact;
class IRCIdentity;
class KopeteMessage;
class KSParser;

struct whoIsInfo;

class IRCContact : public KopeteContact
{
	Q_OBJECT
	
	public:
		IRCContact(IRCIdentity *identity, const QString &nick, KopeteMetaContact *metac);
		~IRCContact();

		// Checks a message for server commands
		bool processMessage( const KopeteMessage & );

		// Nickname stuff
		void setNickName(const QString &nickname) { mNickName = nickname; }
		const QString &nickName() const { return mNickName; }

		virtual KopeteMessageManager* manager( bool canCreate = false );
		virtual const QString caption() const;

		KopeteContact *locateUser( const QString &nickName );

	signals:
		void endSession();

	private slots:
		void slotMessageManagerDestroyed();
		void slotSendMsg(KopeteMessage &message, KopeteMessageManager *);
		void slotNewMessage(const QString &originating, const QString &target, const QString &message);
		void slotNewAction(const QString &originating, const QString &target, const QString &message);
		void slotNewWhoIsUser(const QString &nickname, const QString &username, const QString &hostname, const QString &realname);
		void slotNewWhoIsServer(const QString &nickname, const QString &server, const QString &serverInfo);
		void slotNewWhoIsOperator(const QString &nickname);
		void slotNewWhoIsIdle(const QString &nickname, unsigned long seconds);
		void slotNewWhoIsChannels(const QString &nickname, const QString &channel);
		void slotWhoIsComplete(const QString &nickname);
		void slotNewNickChange( const QString &oldnickname, const QString &newnickname);
		void slotNewCtcpReply(const QString &type, const QString &target, const QString &messageReceived);
		void slotUserDisconnected( const QString &nickname, const QString &reason);

	protected:
		QPtrList<KopeteContact> mContact;
		QPtrList<KopeteContact> mMyself;

		QMap<QString, whoIsInfo*> mWhoisMap;
		KopeteMetaContact *mMetaContact;
		KIRC *mEngine;
		KopeteMessageManager *mMsgManager;
		IRCIdentity *mIdentity;
		QString mNickName;
		KSParser *mParser;


};

#endif
