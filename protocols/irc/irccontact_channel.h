/*
    ircchannelcontact.h - IRC Channel Contact

    Copyright (c) 2002      by Nick Betcher <nbetcher@kde.org>
    Copyright (c) 2003      by Jason Keirstead <jason@keirstead.org>

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

#ifndef IRCCONTACT_CHANNEL_H
#define IRCCONTACT_CHANNEL_H

#include "irccontact.h"
#include <QList>

class QAction;
class KActionMenu;
class KCodecAction;
class KToggleAction;

class IRCAccount;

namespace Kopete {
class MetaContact;
}

/**
 * @author Jason Keirstead <jason@keirstead.org>
 *
 * This class is the @ref Kopete::Contact object representing IRC Channels, not users.
 * It is derrived from IRCContact where much of its functionality is shared with @ref IRCUserContact.
 */
class IRCChannelContact : public IRCContact
{
    friend class IRCSignalMapper;

public:
    IRCChannelContact(IRCAccount *, KIRC::EntityPtr entity, Kopete::MetaContact *metac);
    ~IRCChannelContact();

    /**
     * Returns the current topic for this channel.
     */
    const QString &topic() const
    {
        return mTopic;
    }

    /* Set password for a channel */
    void setPassword(const QString &password)
    {
        mPassword = password;
    }

    /* Get password for a channel */
    const QString &password() const
    {
        return mPassword;
    }

    /**
     * Returns if a mode is enabled for this channel.
     * @param mode The mode you want to check ( 't', 'n', etc. )
     * @param value This is a pointer to a QString which is set to
     * the value of the mode if it has one. Example, the mode 'l' or
     * the mode 'k'. If the mode has no such value then the pointer
     * is always returned null.
     */
    bool modeEnabled(QChar mode, QString *value = 0);

    // Kopete::Contact stuff
    virtual QList<QAction *> *customContextMenuActions();
    virtual const QString caption() const;

    //Methods handled by the signal mapper
    void userJoinedChannel(const QString &user);
    void userPartedChannel(const QString &user, const QString &reason);
    void userKicked(const QString &nick, const QString &nickKicked, const QString &reason);
    void channelTopic(const QString &topic);
    void channelHomePage(const QString &url);
    void topicChanged(const QString &nick, const QString &newtopic);
    void topicUser(const QString &nick, const QDateTime &time);
    void namesList(const QStringList &nicknames);
    void endOfNames();
    void incomingModeChange(const QString &nick, const QString &mode);
    void incomingChannelMode(const QString &mode, const QString &params);
    void failedChankey();
    void failedChanBanned();
    void failedChanInvite();
    void failedChanFull();

public slots:
    void updateStatus();

    /**
     * Sets the topic of this channel
     * @param topic The topic you want set
     */
    void setTopic(const QString &topic = QString());

    /**
     * Sets or unsets a mode on this channel
     * @param mode The full text of the mode change you want performed
     */
    void setMode(const QString &mode = QString());

    void part();

    void join();

protected slots:
    void chatSessionDestroyed();

    virtual void initConversation();

private slots:
    void slotIncomingUserIsAway(const QString &nick, const QString &reason);
    void slotModeChanged();
    void slotAddNicknames();
    void slotConnectedToServer();
    void slotUpdateInfo();
    void slotHomepage();
    void slotChannelListed(const QString &channel, uint members, const QString &topic);

private:
    // QAction stuff:
    QAction *actionJoin;
    QAction *actionPart;
    QAction *actionTopic;
    QAction *actionHomePage;
    KActionMenu *actionModeMenu;
    KCodecAction *codecAction;

    KToggleAction *actionModeT;
    KToggleAction *actionModeN;
    KToggleAction *actionModeS;
    KToggleAction *actionModeI;
    KToggleAction *actionModeP;
    KToggleAction *actionModeM;
    KToggleAction *actionModeB;

    QString mTopic;
    QString mPassword;
    QStringList mJoinedNicks;
    QMap<QString, bool> modeMap;
    QTimer *mInfoTimer;

    void toggleMode(QChar mode, bool enabled, bool update);
};

#endif // IRCCONTACT_CHANNEL_H
