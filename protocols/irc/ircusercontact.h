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

class QTimer;

class KActionCollection;
class KAction;
class KActionMenu;

class IRCContactManager;
class IRCChannelContact;

/**
 * @author Jason Keirstead <jason@keirstead.org
 *
 * This class is the @ref KopeteContact object representing IRC Users, not channels.
 * It is derrived from IRCContact where much of its functionality is shared with @ref IRCChannelContact.
 */
class IRCUserContact
	: public IRCContact
{
	Q_OBJECT

public:
	// This class provides a KopeteContact for each user on the channel.
	IRCUserContact(IRCContactManager *, const QString &nickname, KopeteMetaContact *mc);

	// KopeteContact stuff
	virtual KActionCollection *customContextMenuActions();
	virtual const QString caption() const;

private slots:
	virtual void updateStatus();
public:
	void setAway(bool isAway);

private slots:
	void slotOp();
	void slotDeop();
	void slotVoice();
	void slotDevoice();
	void slotCtcpPing();
	void slotCtcpVersion();
	void slotBanHost();
	void slotBanUserHost();
	void slotBanDomain();
	void slotBanUserDomain();
	void slotKick();
	void slotIncomingModeChange(const QString &nick, const QString &channel, const QString &mode);
	void slotUserOnline(const QString &nick);
	void slotUserOffline();

	virtual void slotUserInfo();

protected slots:
	virtual void privateMessage(IRCContact *from, IRCContact *to, const QString &message);
	virtual void action(IRCContact *from, IRCContact *to, const QString &action);

private:
	KActionCollection *mCustomActions;
	KActionMenu *actionModeMenu;
	KActionMenu *actionCtcpMenu;
	KAction *actionKick;
	KActionMenu *actionBanMenu;
	QTimer *mOnlineTimer;

	bool m_isAway;
	bool m_isOnline;

	void contactMode(const QString &mode);
};

#endif

// vim: set noet ts=4 sts=4 tw=4:

