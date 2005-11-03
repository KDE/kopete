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

#include "kopetechatsessionmanager.h"
#include "irccontact.h"
#include "kopeteonlinestatus.h"

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
	bool isIdentified;
	bool away;
	bool online;
	uint hops;
	QDateTime lastOnline;
	QTime lastUpdate;
};

/**
 * @author Jason Keirstead <jason@keirstead.org
 *
 * This class is the @ref Kopete::Contact object representing IRC Users, not channels.
 * It is derrived from IRCContact where much of its functionality is shared with @ref IRCChannelContact.
 */
class IRCUserContact : public IRCContact
{
	Q_OBJECT

public:
	// This class provides a Kopete::Contact for each user on the channel.
	IRCUserContact(IRCContactManager *, const QString &nickname, Kopete::MetaContact *mc);

	// Kopete::Contact stuff
	virtual QPtrList<KAction> *customContextMenuActions( Kopete::ChatSession *manager );
	virtual const QString caption() const;

	void setAway(bool isAway);

	QString formattedName() const;

	//Methods handled by the signal mapper
	void incomingUserIsAway(const QString &message );
	void userOnline();
	void newAction( const QString &from, const QString &action );
	void newWhoIsUser(const QString &username, const QString &hostname, const QString &realname);
	void newWhoIsServer(const QString &server, const QString &serverInfo);
	void newWhoIsOperator();
	void newWhoIsIdentified();
	void newWhoIsIdle(unsigned long seconds);
	void newWhoIsChannels(const QString &channel);
	void whoIsComplete();
	void whoWasComplete();
	void newWhoReply( const QString &channel, const QString &user, const QString &host,
		const QString &server, bool away, const QString &flags, uint hops,
		const QString &realName );

public slots:
	/** \brief Updates online status for channels based on current internal status.
	 */
	virtual void updateStatus();

	virtual void sendFile(const KURL &sourceURL, const QString&, unsigned int);

protected slots:
	virtual void privateMessage(IRCContact *from, IRCContact *to, const QString &message);

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
	void slotUserOffline();

	void slotBanHostOnce();
	void slotBanUserHostOnce();
	void slotBanDomainOnce();
	void slotBanUserDomainOnce();

	virtual void slotUserInfo();

	//This can't be handled by the contact manager since
	void slotIncomingModeChange(const QString &nick, const QString &channel, const QString &mode);

private:
	enum bitAdjustment { RemoveBits, AddBits };
	void adjustInternalOnlineStatusBits(IRCChannelContact *channel, unsigned statusAdjustment, bitAdjustment adj);

	void contactMode(const QString &mode);
	void updateInfo();

	KActionMenu *actionModeMenu;
	KActionMenu *actionCtcpMenu;
	KAction *actionKick;
	KActionMenu *actionBanMenu;
	KCodecAction *codecAction;
	Kopete::ChatSession *mActiveManager;
	QTimer *mOnlineTimer;
	IRCUserInfo mInfo;
};

#endif

// vim: set noet ts=4 sts=4 tw=4:
