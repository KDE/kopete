
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

#include <qpushbutton.h>
#include <qlineedit.h>
#include "jabberaccount.h"
#include "dlgjabberchatjoin.h"

dlgJabberChatJoin::dlgJabberChatJoin (JabberAccount *account, QWidget * parent, const char *name)
									: dlgChatJoin (parent, name)
{

	m_account = account;

	connect (buttonOk, SIGNAL (clicked ()), this, SLOT (slotDialogDone ()));

}

void dlgJabberChatJoin::slotDialogDone ()
{

	if(!m_account->isConnected())
	{
		m_account->errorConnectFirst();
		return;
	}

	// send the join request
	m_account->client()->groupChatJoin(leServer->text(), leRoom->text(), leNick->text());

}

dlgJabberChatJoin::~dlgJabberChatJoin ()
{
}

#include "dlgjabberchatjoin.moc"
