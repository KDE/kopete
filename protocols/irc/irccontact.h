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

#include "kirc.h"
#include "kopetecontact.h"

class KopeteMessageManager;
class KopeteMetaContact;
class IRCIdentity;
class KopeteMessage;

class IRCContact : public KopeteContact
{
	Q_OBJECT
	
	public:
		IRCContact(IRCIdentity *identity, const QString &nick, KopeteMetaContact *metac);

		// Checks a message for server commands
		bool processMessage( const KopeteMessage & );

		// Nickname stuff
		void setNickName(const QString &nickname) { mNickName = nickname; }
		const QString &nickName() { return mNickName; }

		virtual KopeteMessageManager* manager( bool canCreate = false );
		virtual const QString &caption() const;

	signals:
		void endSession();

	private slots:
		void slotMessageManagerDestroyed();
		void slotSendMsg(KopeteMessage &message, KopeteMessageManager *);
		void slotNewMessage(const QString &originating, const QString &target, const QString &message);
		void slotNewAction(const QString &originating, const QString &target, const QString &message);
		void slotNewWhois(const QString &nickname, const QString &username, const QString &hostname, const QString &realname);

	protected:
		QPtrList<KopeteContact> mContact;
		QPtrList<KopeteContact> mMyself;

		KopeteMetaContact *mMetaContact;
		KIRC *mEngine;
		KopeteMessageManager *mMsgManager;
		IRCIdentity *mIdentity;
		QString mNickName;

		KopeteContact *locateUser( const QString &nickName );
};

#endif
