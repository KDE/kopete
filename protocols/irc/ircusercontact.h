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

#include "kopetemessagemanagerfactory.h"
#include "irccontact.h"

class IRCIdentity;
class KActionCollection;
class KAction;
class KActionMenu;
class IRCChannelContact;
class QTimer;

class IRCUserContact : public IRCContact
{
	Q_OBJECT

	public:
		// This class provides a KopeteContact for each user on the channel.
		IRCUserContact(IRCIdentity *, const QString &nickname,KopeteMetaContact *mc  );
		~IRCUserContact();

		// Userclass stuff
		void setUserclass(const QString &channel, KIRC::UserClass userclass) { mUserClassMap[channel.lower()] = userclass; }
		const KIRC::UserClass userclass( const QString &channel ) const { return  mUserClassMap[channel.lower()]; }

		// KopeteContact stuff
		virtual QString statusIcon() const;
		virtual KActionCollection *customContextMenuActions() { return mCustomActions; };
		virtual const QString caption() const;
		virtual KopeteMessageManager* manager( bool canCreate = false );

		void addChannel( const QString &channel ) { mChannels.append( channel.lower() ); };
		void removeChannel( const QString &channel ) { mChannels.remove( channel ); };
		const bool inChannel( const QString &channel ) const { return mChannels.contains( channel ); };

	private slots:
		void slotMessageManagerDestroyed();
		virtual void slotUserInfo();
		void slotOp();
		void slotDeop();
		void slotVoice();
		void slotDevoice();
		void slotCtcpPing();
		void slotCtcpVersion();
		void slotUserOffline();
		void slotBanHost();
		void slotBanUserHost();
		void slotBanDomain();
		void slotBanUserDomain();
		void slotKick();
		void slotNewPrivMessage(const QString &originating, const QString &target, const QString &message);
		void slotIncomingModeChange(const QString &nick, const QString &channel, const QString &mode);
		void slotUserOnline( const QString &nick );

	private:
		KActionCollection *mCustomActions;
		KActionMenu *actionModeMenu;
		KActionMenu *actionCtcpMenu;
		KAction *actionKick;
		KActionMenu *actionBanMenu;
		QTimer *mOnlineTimer;
		QStringList mChannels;
		QMap<QString,KIRC::UserClass> mUserClassMap;

		void contactMode( const QString &mode );
};

#endif
