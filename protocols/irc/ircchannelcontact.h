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

#ifndef IRCCHANNELCONTACT_H
#define IRCCHANNELCONTACT_H

#include "irccontact.h"

class KActionCollection;
class KAction;
class KActionMenu;
class KToggleAction;

class KopeteMetaContact;
class KopeteMessageManager;
class KopeteMessage;
class KopeteView;

class IRCAccount;
class IRCContactManager;

/**
 * @author Jason Keirstead <jason@keirstead.org>
 *
 * This class is the @ref KopeteContact object representing IRC Channels, not users.
 * It is derrived from IRCContact where much of its functionality is shared with @ref IRCUserContact.
 */
class IRCChannelContact
	: public IRCContact
{
	Q_OBJECT

public:
	IRCChannelContact(IRCContactManager *, const QString &channel, KopeteMetaContact *metac);
	~IRCChannelContact();

	/**
	 * Returns the current topic for this channel.
	 */
	 const QString &topic() const { return mTopic; };

	/* Set password for a channel */
	void setPassword(const QString &password) { mPassword = password; }
	/* Get password for a channel */
	const QString &password() const { return mPassword; }

	/**
	 * Returns if a mode is enabled for this channel.
	 * @param mode The mode you want to check ( 't', 'n', etc. )
	 * @param value This is a pointer to a QString which is set to
	 * the value of the mode if it has one. Example, the mode 'l' or
	 * the mode 'k'. If the mode has no such value then the pointer
	 * is always returned null.
	 */
	bool modeEnabled( QChar mode, QString *value = 0 );

	// KopeteContact stuff
	virtual KActionCollection *customContextMenuActions();
	virtual const QString caption() const;

public slots:
	virtual void updateStatus();

	/**
	 * Sets the topic of this channel
	 * @param topic The topic you want set
	 */
	void setTopic( const QString &topic = QString::null );

	/**
	 * Sets or unsets a mode on this channel
	 * @param mode The full text of the mode change you want performed
	 */
	void setMode( const QString &mode = QString::null );
	
	void part();

protected slots:
	void messageManagerDestroyed();

	virtual void privateMessage(IRCContact *from, IRCContact *to, const QString &message);
	virtual void action(IRCContact *from, IRCContact *to, const QString &action);

public slots: // should be viewCreated( KopeteView* )
	void slotJoinChannel( KopeteView* );
	void slotFailedChankey(const QString &channame);

private slots:
	void slotConnectedToServer();
	void slotUserJoinedChannel(const QString &, const QString &);
	void slotJoin();
	void slotUserPartedChannel(const QString &user, const QString &channel, const QString &reason);
	void slotChannelTopic(const QString &channel, const QString &topic);
	void slotTopicChanged(const QString &channel, const QString &nick, const QString &newtopic);
	void slotNamesList(const QString &channel, const QStringList &nicknames);
	void slotIncomingModeChange(const QString &nick, const QString &channel, const QString &mode);
	void slotIncomingChannelMode( const QString &channel, const QString &mode, const QString &params );
	void slotModeChanged();
	void slotAddNicknames();


private:
	// KAction stuff:
	KActionCollection *mCustomActions;
	KAction *actionJoin;
	KAction *actionPart;
	KAction *actionTopic;
	KActionMenu *actionModeMenu;

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
	QMap<QString,bool> modeMap;
	void toggleMode( QChar mode, bool enabled, bool update );
};

#endif
