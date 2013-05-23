
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
#include <kmessagebox.h>
#include <qlineedit.h>
#include <qpushbutton.h>

#include "jabberaccount.h"
#include "jabberclient.h"

#include "xmpp_tasks.h"

dlgJabberChatJoin::dlgJabberChatJoin(JabberAccount *account, QWidget* parent) 
: KDialog(parent), m_account(account)
{
	setCaption( i18n("Join Jabber Groupchat") );
	setButtons( KDialog::Cancel | KDialog::User1 );
	setButtonGuiItem( KDialog::User1, KGuiItem( i18n("Join") ) );
	
	QWidget *mainWidget = new QWidget(this);
	m_ui.setupUi(mainWidget);
	setMainWidget(mainWidget);
	
	m_ui.leNick->setText(m_account->client()->client()->user());
	checkDefaultChatroomServer();

	connect(this, SIGNAL(user1Clicked()), this, SLOT(slotJoin()));
	connect(m_ui.pbQuery, SIGNAL(clicked()), this, SLOT(slotQuery()));
	connect(m_ui.tblChatRoomsList, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)), this, SLOT(slotDoubleClick(QTreeWidgetItem*)));
	connect(m_ui.leServer, SIGNAL(textChanged(QString)), this, SLOT(slotCheckData()));
	connect(m_ui.leRoom, SIGNAL(textChanged(QString)), this, SLOT(slotCheckData()));
	connect(m_ui.leNick, SIGNAL(textChanged(QString)), this, SLOT(slotCheckData()));

	slotCheckData();
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

	m_account->client()->joinGroupChat(m_ui.leServer->currentText(), m_ui.leRoom->text(), m_ui.leNick->text());
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
	XMPP::JT_DiscoItems *serviceTask = new JT_DiscoItems(m_account->client()->rootTask());
	connect(serviceTask, SIGNAL (finished()), this, SLOT (slotQueryFinished()));

	serviceTask->get(m_account->server());
	serviceTask->go(true);
}

void dlgJabberChatJoin::slotQueryFinished()
{
	XMPP::JT_DiscoItems *task = (JT_DiscoItems *)sender();
	if (!task->success())
		return;

	const XMPP::DiscoList &list = task->items();
	for (XMPP::DiscoList::ConstIterator it = list.begin(); it != list.end(); ++it)
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
	
	if (task->item().features().canGroupchat() && !task->item().features().isGateway())
	{
		const QString text = m_ui.leServer->currentText();
		const bool wasEmpty = m_ui.leServer->count() == 0;
		m_ui.leServer->addItem(task->item().jid().full());
		// the combobox was empty, and the edit text was not empty,
		// so restore the previous edit text
		if (wasEmpty && !text.isEmpty())
		{
			m_ui.leServer->setEditText(text);
		}
	}
}

void dlgJabberChatJoin::slotQuery()
{
	XMPP::JT_DiscoItems *discoTask = new XMPP::JT_DiscoItems(m_account->client()->rootTask());
	connect (discoTask, SIGNAL(finished()), this, SLOT(slotChatRooomsQueryFinished()));

	m_ui.tblChatRoomsList->clear();

	discoTask->get(m_ui.leServer->currentText());
	discoTask->go(true);
}

void dlgJabberChatJoin::slotChatRooomsQueryFinished()
{
	XMPP::JT_DiscoItems *task = (XMPP::JT_DiscoItems*)sender();
	if (!task->success())
	{
		KMessageBox::queuedMessageBox(this, KMessageBox::Error, i18n("Unable to retrieve the list of chat rooms."),  i18n("Jabber Error"));
		return;
	}

	const XMPP::DiscoList& items = task->items();

	for (XMPP::DiscoList::const_iterator it = items.begin(); it != items.end(); ++it)
	{
		const XMPP::DiscoItem &di = *it;
		if (di.jid().node().isEmpty())
			continue;
		QTreeWidgetItem *item = new QTreeWidgetItem();
		item->setText(0, di.jid().node());
		item->setText(1, di.name());
		m_ui.tblChatRoomsList->addTopLevelItem(item);
	}
	m_ui.tblChatRoomsList->sortItems(0, Qt::AscendingOrder);
}

void dlgJabberChatJoin::slotDoubleClick(QTreeWidgetItem *item)
{
	m_ui.leRoom->setText(item->text(0));
	if (!(m_ui.leServer->currentText().isEmpty() || m_ui.leNick->text().isEmpty()))
	{
		slotJoin();
	}
}

void dlgJabberChatJoin::slotCheckData()
{
	bool enableJoinButton = !(m_ui.leServer->currentText().isEmpty() || m_ui.leRoom->text().isEmpty() || m_ui.leNick->text().isEmpty());
	enableButton(User1, enableJoinButton);
}

// end todo

#include "dlgjabberchatjoin.moc"

