//
// C++ Implementation: 
//
// Description: 
//
//
// Author: Kopete Developers <kopete-devel@kde.org>, (C) 2005
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include <kmessagebox.h>
#include <klocale.h>
#include <kdebug.h>

#include <qtable.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qlabel.h>

#include "dlgjabberchatroomslist.h"
#include "jabberprotocol.h"

dlgJabberChatRoomsList::dlgJabberChatRoomsList(JabberAccount* account, const QString& server, const QString &nick,  QWidget *parent, const char *name) :
dlgChatRoomsList(parent, name),
	m_account(account) , m_selectedRow(-1) ,  m_nick(nick)
{
	if (!server.isNull())
		leServer->setText(server);
	else if(m_account->isConnected())
		leServer->setText(m_account->server());

	m_chatServer = leServer->text();

	// locales
	setCaption(i18n("List Chatrooms"));

	tblChatRoomsList->setLeftMargin (0);
	tblChatRoomsList->setColumnStretchable(0, true);
	tblChatRoomsList->setColumnStretchable(1, true);

	if (!server.isNull())
		slotQuery();
}

dlgJabberChatRoomsList::~dlgJabberChatRoomsList()
{
}

/*$SPECIALIZATION$*/
void dlgJabberChatRoomsList::slotJoin()
{
	if(!m_account->isConnected())
	{
		m_account->errorConnectFirst();
		return;
	}

	if (m_selectedRow >= 0)
	{
		kdDebug (JABBER_DEBUG_GLOBAL) << "join chat room : " <<  m_account->client()->client()->user() << " @ " << tblChatRoomsList->text(m_selectedRow, 0) << " on " << m_chatServer << endl;
		m_account->client()->joinGroupChat(m_chatServer, tblChatRoomsList->text(m_selectedRow, 0), m_nick);
	}
}

void dlgJabberChatRoomsList::slotQuery()
{
	if(!m_account->isConnected())
	{
		m_account->errorConnectFirst();
		return;
	}

	tblChatRoomsList->setNumRows(0);

	XMPP::JT_DiscoItems *discoTask = new XMPP::JT_DiscoItems(m_account->client()->rootTask());
	connect (discoTask, SIGNAL(finished()), this, SLOT(slotQueryFinished()));

	m_chatServer = leServer->text();
	discoTask->get(leServer->text());
	discoTask->go(true);
}

void dlgJabberChatRoomsList::slotQueryFinished()
{
	XMPP::JT_DiscoItems *task = (XMPP::JT_DiscoItems*)sender();
	if (!task->success())
	{
		KMessageBox::queuedMessageBox(this, KMessageBox::Error, i18n("Unable to retrieve the list of chat rooms."),  i18n("Jabber Error"));
		return;
	}

	const XMPP::DiscoList& items = task->items();
	tblChatRoomsList->setNumRows(items.count());

	int row = 0;
	for (XMPP::DiscoList::const_iterator it = items.begin(); it != items.end(); ++it)
	{
		tblChatRoomsList->setText(row, 0, (*it).jid().user());
		tblChatRoomsList->setText(row, 1, (*it).name());
		++row;
	}
}

void dlgJabberChatRoomsList::slotDoubleClick(int row, int /*col*/, int /*button*/, const QPoint& /*mousePos*/)
{
	m_selectedRow = row;
	slotJoin();
}

void dlgJabberChatRoomsList::slotClick(int row, int /*col*/, int /*button*/, const QPoint& /*mousePos*/)
{
	m_selectedRow = row;
}

#include "dlgjabberchatroomslist.moc"

