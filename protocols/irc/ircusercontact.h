/*
    ircusercontact.h - IRC User Contact

    Copyright (c) 2002      by Nick Betcher <nbetcher@kde.org>
    Copyright (c) 2003      by Jason Keirstead <jason@keirstead.org

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

class IRCAccount;
class KActionCollection;
class KAction;
class KActionMenu;
class IRCChannelContact;
class QTimer;

/**
 * @author Jason Keirstead <jason@keirstead.org
 *
 * This class is the @ref KopeteContact object representing IRC Users, not channels.
 * It is derrived from IRCContact where much of its functionality is shared with @ref IRCChannelContact.
 */
class IRCUserContact : public IRCContact
{
	Q_OBJECT

public:
	// This class provides a KopeteContact for each user on the channel.
	IRCUserContact(IRCAccount *, const QString &nickname,KopeteMetaContact *mc  );

	/**
	 * This sets this UserClass of the contact, to either Op, Voiced, or Normal.
	 * This setting invluences menu options of the KopeteAccount::myself() user, as well
	 * as alters the user icons in the chat members display.
	 */
	void setUserclass(const QString &channel, KIRC::UserClass userclass) { mUserClassMap[channel.lower()] = userclass; }

	/**
	 * Returns the user class of this contact
	 */
	const KIRC::UserClass userclass( const QString &channel ) const { return  mUserClassMap[channel.lower()]; }

	// KopeteContact stuff
	virtual KActionCollection *customContextMenuActions() { return mCustomActions; };
	virtual const QString caption() const;
	virtual KopeteMessageManager* manager( bool canCreate = false );

private slots:
	void slotMessageManagerDestroyed();
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
	void slotIncomingModeChange(const QString &nick, const QString &channel, const QString &mode);
	void slotUserOnline( const QString &nick );

	virtual void slotUserInfo();

private:
	KActionCollection *mCustomActions;
	KActionMenu *actionModeMenu;
	KActionMenu *actionCtcpMenu;
	KAction *actionKick;
	KActionMenu *actionBanMenu;
	QTimer *mOnlineTimer;
	QMap<QString,KIRC::UserClass> mUserClassMap;

	void contactMode( const QString &mode );
};

#endif

// vim: set noet ts=4 sts=4 tw=4:

