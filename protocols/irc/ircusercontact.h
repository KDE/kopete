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
class KCodecAction;

class IRCContactManager;
class IRCChannelContact;

struct IRCUserInfo
{
	QString userName;
	QString hostName;
	QString realName;
	QString serverName;
	QString serverInfo;
	QString flags;
	QStringList channels;
	unsigned long idle;
	bool isOperator;
	bool away;
	uint hops;
};

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
	IRCUserContact(IRCContactManager *, const QString &nickname, KopeteMetaContact *mc);

	// KopeteContact stuff
	virtual QPtrList<KAction> *customContextMenuActions( KopeteMessageManager *manager );
	virtual const QString caption() const;

	void setAway(bool isAway);

	QString formattedName() const;

protected slots:
	virtual void privateMessage(IRCContact *from, IRCContact *to, const QString &message);
	virtual void action(IRCContact *from, IRCContact *to, const QString &action);

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
	void slotIncomingUserIsAway( const QString &nick, const QString &message );
	void slotUserOnline(const QString &nick);
	void slotUserOffline();

	void slotNewWhoIsUser(const QString &nickname, const QString &username, const QString &hostname, const QString &realname);
	void slotNewWhoIsServer(const QString &nickname, const QString &server, const QString &serverInfo);
	void slotNewWhoIsOperator(const QString &nickname);
	void slotNewWhoIsIdle(const QString &nickname, unsigned long seconds);
	void slotNewWhoIsChannels(const QString &nickname, const QString &channel);
	void slotWhoIsComplete(const QString &nickname);
	void slotNewWhoReply( const QString &channel, const QString &user, const QString &host,
		const QString &server, const QString &nick, bool away, const QString &flags, uint hops,
		const QString &realName );

	virtual void updateStatus();
	virtual void slotUserInfo();

private:
	KActionMenu *actionModeMenu;
	KActionMenu *actionCtcpMenu;
	KAction *actionKick;
	KActionMenu *actionBanMenu;
	KCodecAction *codecAction;
	KopeteMessageManager *mActiveManager;
	QTimer *mOnlineTimer;
	IRCUserInfo mInfo;

	bool m_isAway;
	bool m_isOnline;

	void contactMode(const QString &mode);
	void updateInfo();
};

#endif

// vim: set noet ts=4 sts=4 tw=4:

