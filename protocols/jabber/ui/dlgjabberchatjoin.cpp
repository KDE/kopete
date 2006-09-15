
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

#include <kdebug.h>
#include <klocale.h>
#include <qlineedit.h>
#include <qpushbutton.h>

#include "jabberaccount.h"
#include "jabberclient.h"
#include "dlgjabberchatroomslist.h"

#include "dlgjabberchatjoin.h"

dlgJabberChatJoin::dlgJabberChatJoin(JabberAccount *account, QWidget* parent) 
: KDialog(parent), m_account(account)
{
	setCaption( i18n("Join Jabber Groupchat") );
	setButtons( KDialog::User1 | KDialog::User2 );
	setButtonGuiItem( KDialog::User1, KGuiItem( i18n("Join") ) );
	setButtonGuiItem( KDialog::User2, KGuiItem( i18n("Browser") ) );
	
	QWidget *mainWidget = new QWidget(this);
	m_ui.setupUi(mainWidget);
	setMainWidget(mainWidget);
	
	m_ui.leNick->setText(m_account->client()->client()->user());
	checkDefaultChatroomServer();

	connect(this, SIGNAL(user1Clicked()), this, SLOT(slotJoin()));
	connect(this, SIGNAL(user2Clicked()), this, SLOT(slotBowse()));
}

dlgJabberChatJoin::~dlgJabberChatJoin()
{
}

/*$SPECIALIZATION$*/
void dlgJabberChatJoin::slotJoin()
{
	if(!m_account->isConnected())
	{
		m_account->errorConnectFirst();
		return;
	}

	m_account->client()->joinGroupChat(m_ui.leServer->text(), m_ui.leRoom->text(), m_ui.leNick->text());
	accept();
}

void dlgJabberChatJoin::slotBowse()
{
	if(!m_account->isConnected())
	{
		m_account->errorConnectFirst();
		return;
	}

	dlgJabberChatRoomsList *crl = new dlgJabberChatRoomsList(m_account, m_ui.leServer->text() , m_ui.leNick->text());
	crl->show();
	accept();
}

/*
	TODO : Used to look for the default chat server,
	this is duplicate with dlgjabberservices.cpp
	should be merged elsewhere !
*/
//	JabberAccount *m_account;
//	XMPP::JT_GetServices * serviceTask;

void dlgJabberChatJoin::checkDefaultChatroomServer()
{
	XMPP::JT_GetServices *serviceTask = new XMPP::JT_GetServices(m_account->client()->rootTask());
	connect(serviceTask, SIGNAL (finished()), this, SLOT (slotQueryFinished()));

	serviceTask->get(m_account->server());
	serviceTask->go(true);
}

void dlgJabberChatJoin::slotQueryFinished()
{
	XMPP::JT_GetServices *task = (XMPP::JT_GetServices*)sender();
	if (!task->success ())
		return;
	
	if(!m_ui.leServer->text().isEmpty())
	{  //the user already started to type the server manyally. abort auto-detect
		return;
	}

	for (XMPP::AgentList::const_iterator it = task->agents().begin(); it != task->agents().end(); ++it)
	{
		XMPP::JT_DiscoInfo *discoTask = new XMPP::JT_DiscoInfo(m_account->client()->rootTask());
		connect(discoTask, SIGNAL (finished()), this, SLOT (slotDiscoFinished()));

		discoTask->get((*it).jid().full());
		discoTask->go(true);
	}
}

void dlgJabberChatJoin::slotDiscoFinished()
{
	XMPP::JT_DiscoInfo *task = (XMPP::JT_DiscoInfo*)sender();

	if (!task->success())
		return;
	
	if(!m_ui.leServer->text().isEmpty())
	{  //the user already started to type the server manyally. abort auto-detect
		return;
	}


	if (task->item().features().canGroupchat() && !task->item().features().isGateway())
	{
		m_ui.leServer->setText(task->item().jid().full());
	}
}

// end todo

#include "dlgjabberchatjoin.moc"

