/*
    ircusercontact.h - IRC User Contact

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

#ifndef IRCUSERCONTACT_H
#define IRCUSERCONTACT_H

#include "irccontact.h"

class IRCIdentity;
class KActionCollection;
class KAction;
class KActionMenu;
class IRCChannelContact;

class IRCUserContact : public IRCContact
{
	Q_OBJECT

	public:
		// This class provides a KopeteContact for each user on the channel.
		IRCUserContact(IRCIdentity *, const QString &nickname, KIRC::UserClass, KopeteMetaContact * = 0L);

		// Userclass stuff
		void setUserclass(KIRC::UserClass userclass) { mUserclass = userclass; }
		KIRC::UserClass userclass() { return mUserclass; }

		// KopeteContact stuff
		virtual QString statusIcon() const;
		virtual KActionCollection *customContextMenuActions() { return mCustomActions; };
		virtual const QString caption() const;
		uint conversations() const { return mConversations; };
		void setConversations( uint conversations ) { mConversations = conversations; };

	private slots:
		void slotWhois();
		void slotOp();
		void slotDeop();
		void slotVoice();
		void slotDevoice();
		void slotCtcpPing();
		void slotCtcpVersion();
		void slotNewPrivMessage(const QString &originating, const QString &target, const QString &message);
		void slotIncomingModeChange(const QString &nick, const QString &channel, const QString &mode);

	private:
		KIRC::UserClass mUserclass;

		KActionCollection *mCustomActions;
		KActionMenu *actionModeMenu;
		KAction *actionOp;
		KAction *actionDeop;
		KAction *actionVoice;
		KAction *actionDevoice;
		KActionMenu *actionCtcpMenu;
		KAction *actionCtcpPing;
		KAction *actionCtcpVersion;
		KActionMenu *actionKick;
		KActionMenu *actionBan;
		KAction *actionWhois;
		uint mConversations;

		void contactMode( const QString &mode );
};

#endif
