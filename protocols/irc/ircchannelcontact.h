/*
    ircchannelcontact.h - IRC Channel Contact

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

#ifndef IRCCHANNELCONTACT_H
#define IRCCHANNELCONTACT_H

#include "irccontact.h"

class IRCIdentity;
class KopeteMetaContact;
class KActionCollection;
class KAction;
class KActionMenu;
class KToggleAction;
class KopeteMessageManager;
class KopeteMessage;

class IRCChannelContact : public IRCContact
{
	Q_OBJECT

	public:
		IRCChannelContact(IRCIdentity *, const QString &channel, KopeteMetaContact *metac);

		~IRCChannelContact();

		// START: Virtual reimplmentations from KopeteContact, see kopetecontact.h:
		virtual bool isReachable();
		virtual KActionCollection *customContextMenuActions();
		virtual QString statusIcon() const;
		virtual const QString caption() const;
		const QString &topic() const { return mTopic; };
		// FINISH

	public slots:
		void setTopic( const QString &topic = QString::null );
		void setMode( const QString &mode = QString::null );

	private slots:
		void slotUserJoinedChannel(const QString &, const QString &);
		void slotJoin();
		void slotPart();
		void slotUserPartedChannel(const QString &user, const QString &channel, const QString &reason);
		void slotChannelTopic(const QString &channel, const QString &topic);
		void slotTopicChanged(const QString &channel, const QString &nick, const QString &newtopic);
		void slotNamesList(const QString &channel, const QString &nickname, const int);
		void slotIncomingModeChange(const QString &nick, const QString &channel, const QString &mode);
		void slotModeChanged();

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
};

#endif
