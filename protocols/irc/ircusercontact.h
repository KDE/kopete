/***************************************************************************
                          ircusercontact.h  -  description
                             -------------------
    begin                : Thu Feb 20 2003
    copyright            : (C) 2003 by nbetcher
    email                : nbetcher@kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef IRCUSERCONTACT_H
#define IRCUSERCONTACT_H

#include "irccontact.h"

class IRCIdentity;
class KActionCollection;
class KAction;
class KActionMenu;

class IRCUserContact : public IRCContact
{
	Q_OBJECT

	public:
		// This class provides a KopeteContact for each user on the channel.
		IRCUserContact(IRCIdentity *, const QString &nickname, KIRC::UserClass);

		// Userclass stuff
		void setUserclass(KIRC::UserClass userclass) { mUserclass = userclass; }
		KIRC::UserClass userclass() { return mUserclass; }

		// KopeteContact stuff
		virtual QString statusIcon() const;
		virtual KActionCollection *customContextMenuActions() { return mCustomActions; };
		virtual const QString caption() const;

	private slots:
		void slotWhois();

	private:
		KIRC::UserClass mUserclass;

		KActionCollection *mCustomActions;
		KAction *actionOp;
		KAction *actionDeop;
		KAction *actionVoice;
		KAction *actionDevoice;
		KActionMenu *actionKick;
		KActionMenu *actionBan;
		KAction *actionWhois;
};

#endif
