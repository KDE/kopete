
/***************************************************************************
                          dlgjabberchatjoin.cpp  -  description
                             -------------------
    begin                : Fri Dec 13 2002
    copyright            : (C) 2002-2003 by Till Gerken <till@tantalo.net>
    email                : kopete-devel@kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "dlgjabberchatjoin.h"

#include <kdebug.h>
#include <klocale.h>
#include <kdialogbase.h>
#include <qlineedit.h>

#include "jabberaccount.h"
#include "jabberclient.h"

#include "dlgchatjoin.h"

dlgJabberChatJoin::dlgJabberChatJoin (JabberAccount *account, QWidget * parent, const char *name)
									: KDialogBase (parent, name, false,
												   i18n("Join Jabber Groupchat"),
												   KDialogBase::Ok | KDialogBase::Cancel)
{

	m_account = account;

	setMainWidget ( new dlgChatJoin ( this ) );

}

void dlgJabberChatJoin::slotOk ()
{

	if(!m_account->isConnected())
	{
		m_account->errorConnectFirst();
		return;
	}

	dlgChatJoin *widget = dynamic_cast<dlgChatJoin *>(mainWidget ());

	// send the join request
	m_account->client()->joinGroupChat ( widget->leServer->text (), widget->leRoom->text (), widget->leNick->text () );

	delete this;

}

void dlgJabberChatJoin::slotCancel ()
{

	delete this;

}

#include "dlgjabberchatjoin.moc"
