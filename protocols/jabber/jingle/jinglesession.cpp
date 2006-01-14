/*
    jinglesession.h - Define a Jingle session.

    Copyright (c) 2006      by Michaël Larouche     <michael.larouche@kdemail.net>

    Kopete    (c) 2001-2006 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/
#include "jinglesession.h"

#include <kdebug.h>

#include "jabberaccount.h"
#include "jabberprotocol.h"

class JingleSession::Private
{
public:
	Private(JabberAccount *t_account, const JidList &t_peers)
	 :  peers(t_peers), account(t_account)
	{}

	XMPP::Jid myself;
	JidList peers;
	JabberAccount *account;
};

JingleSession::JingleSession(JabberAccount *account, const JidList &peers)
 : QObject(account, 0), d(new Private(account, peers))
{
	d->myself = account->client()->jid();
}

JingleSession::~JingleSession()
{
	kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << endl;
	delete d;
}

const XMPP::Jid &JingleSession::myself() const
{
	return d->myself;
}

const JingleSession::JidList &JingleSession::peers() const
{
	return d->peers;
}

JingleSession::JidList &JingleSession::peers()
{
	return d->peers;
}
JabberAccount *JingleSession::account()
{
	return d->account;
}

void JingleSession::sendStanza(const QString &stanza)
{
	account()->client()->send( stanza );
}

#include "jinglesession.moc"
