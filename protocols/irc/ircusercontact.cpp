/***************************************************************************
                          ircchannelcontact.cpp  -  description
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

#include "ircusercontact.h"
#include "ircidentity.h"

#include "kirc.h"

#include <kdebug.h>
#include <klocale.h>
#include <kaction.h>

IRCUserContact::IRCUserContact(IRCIdentity *identity, const QString &nickname, KIRC::UserClass userclass)
	: IRCContact( identity, nickname, 0L )
{
	mUserclass = userclass;
	mNickName = nickname;
	setDisplayName(mNickName);

	mCustomActions = new KActionCollection(this);
	actionOp = new KAction(i18n("&Op"), 0, this, SLOT(slotOp()), mCustomActions, "actionOp");
	actionDeop = new KAction(i18n("&Deop"), 0, this, SLOT(slotDeop()), mCustomActions, "actionDeop");
	actionVoice = new KAction(i18n("&Voice"), 0, this, SLOT(slotOp()), mCustomActions, "actionVoice");
	actionDevoice = new KAction(i18n("Devoice"), 0, this, SLOT(slotDeop()), mCustomActions, "actionDevoice");
	actionWhois = new KAction(i18n("Whois"), 0, this, SLOT(slotWhois()), mCustomActions, "actionWhois");
}

QString IRCUserContact::statusIcon() const
{
	if (mUserclass == KIRC::Operator)
		return "irc_op";
	else if (mUserclass == KIRC::Voiced)
		return "irc_voice";

	return "irc_normal";
}

void IRCUserContact::slotWhois()
{
	mEngine->whoisUser( mNickName );
}

#include "ircusercontact.moc"
