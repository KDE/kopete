/*
    irccontact.h - IRC Contact

    Copyright (c) 2002      by Nick Betcher <nbetcher@kde.org>
    Copyright (c) 2003      by Jason Keirstead <jason@keirstead.org>
    Copyright (c) 2003      by Jason Keirstead <michel.hermier@wanadoo.fr>

    Kopete    (c) 2002-2003 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef IRCCONTACT_H
#define IRCCONTACT_H

#include <qptrlist.h>
#include <qmap.h>

#include "kirc.h"
#include "kopetecontact.h"
#include "kopetemessage.h"

class QTextCodec;
class KopeteMessageManager;
class KopeteMetaContact;
class KopeteView;

class IRCProtocol;
class IRCAccount;
class IRCContactManager;

/**
 * @author Jason Keirstead <jason@keirstead.org>
 * @author Michel Hermier <michel.hermier@wanadoo.fr>
 *
 * This class is the base class for @ref IRCUserContact and @ref IRCChannelContact.
 * Common routines and signal connections that are required for both types of
 * contacts reside here, to avoid code duplication between these two classes.
 */
class IRCContact : public KopeteContact
{
	Q_OBJECT

	public:
		IRCContact(IRCContactManager *contactManager, const QString &nick, KopeteMetaContact *metac, const QString& icon = QString::null);
		~IRCContact();

		/**
		* Sets the nickname of this contact. The nickname is distinct from the displayName
		* in case trackNameChanges is disabled.
		*/
		void setNickName(const QString &nickname) { m_nickName = nickname; setDisplayName(nickname); }

		/**
		* Returns the nickname / channel name
		*/
		const QString &nickName() const { return m_nickName; }

		/**
		* This function attempts to find the nickname specified within the current chat
		* session. Returns a pointer to that IRCUserContact, or 0L if the user does not
		* exist in this session. More useful for channels. Calling IRCChannelContact::locateUser()
		* for example tells you if a user is in a certain channel.
		*/
		KopeteContact *locateUser( const QString &nickName );

		virtual bool isReachable();

		/**
		* return true if the contact is in a chat. false if the contact is in no chats
		* that loop over all manager, and checks the presence of the user
		*/
		bool isChatting() const;

		virtual const QString caption() const = 0;

		virtual KopeteMessageManager *manager(bool canCreate = true);

		virtual void appendMessage( KopeteMessage & );

		const QTextCodec *codec();

		KopeteView *view();

	public slots:
		void setCodec( const QTextCodec *codec );

	protected slots:
		virtual void slotSendMsg(KopeteMessage &message, KopeteMessageManager *);

		virtual void messageManagerDestroyed();

		void slotNewNickChange( const QString &oldnickname, const QString &newnickname);
		void slotUserDisconnected( const QString &nickname, const QString &reason);


		virtual void slotDeleteContact();
		virtual void privateMessage(IRCContact *from, IRCContact *to, const QString &message);
		virtual void action(IRCContact *from, IRCContact *to, const QString &action);
		virtual void updateStatus() = 0;
		virtual void initConversation() {};

	protected:

		IRCProtocol *m_protocol;
		IRCAccount *m_account;
		KIRC *m_engine;
		KopeteMetaContact *m_metaContact;
		QString m_nickName;
		KopeteMessageManager *m_msgManager;
		bool m_isConnected;

		QPtrList<KopeteContact> mMyself;
		KopeteMessage::MessageDirection execDir;
};

#endif
