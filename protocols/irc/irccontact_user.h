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

#ifndef IRCCONTACT_USER_H
#define IRCCONTACT_USER_H

#include "kopetechatsessionmanager.h"
#include "irccontact.h"
#include "kopeteonlinestatus.h"
#include <QList>

class QTimer;

class QAction;
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
public:
    // This class provides a Kopete::Contact for each user on the channel.
    IRCUserContact(IRCContactManager *, const QString &nickname, Kopete::MetaContact *mc);

    // Kopete::Contact stuff
    virtual QList<QAction *> *customContextMenuActions(Kopete::ChatSession *manager);
    virtual const QString caption() const;

    void setAway(bool isAway);

    QString formattedName() const;

    //Methods handled by the signal mapper
    void incomingUserIsAway(const QString &message);
    void userOnline();
    void newWhoIsUser(const QString &username, const QString &hostname, const QString &realname);
    void newWhoIsServer(const QString &server, const QString &serverInfo);
    void newWhoIsOperator();
    void newWhoIsIdentified();
    void newWhoIsIdle(unsigned long seconds);
    void newWhoIsChannels(const QString &channel);
    void whoIsComplete();
    void whoWasComplete();
    void newWhoReply(const QString &channel, const QString &user, const QString &host, const QString &server, bool away, const QString &flags, uint hops, const QString &realName);

public slots:
    virtual void updateStatus();

    virtual void sendFile(const KUrl &sourceURL, const QString &, unsigned int);

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

    virtual void slotUserInfo();

    //This can't be handled by the contact manager since
    void slotIncomingModeChange(const QString &nick, const QString &channel, const QString &mode);

private:
    void setManagerStatus(IRCChannelContact *channel, int statusAdjustment);

    KActionMenu *actionModeMenu;
    KActionMenu *actionCtcpMenu;
    QAction *actionKick;
    KActionMenu *actionBanMenu;
    KCodecAction *codecAction;
    Kopete::ChatSession *mActiveManager;
    QTimer *mOnlineTimer;
    IRCUserInfo mInfo;

    bool m_isAway;
    bool m_isOnline;

    void contactMode(const QString &mode);
    void updateInfo();
};

#endif // IRCCONTACT_USER_H
