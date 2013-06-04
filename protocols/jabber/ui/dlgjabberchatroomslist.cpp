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
#include "dlgjabberchatroomslist.h"
#include <kmessagebox.h>
#include <klocale.h>
#include <kdebug.h>

#include <qlineedit.h>
#include <qpushbutton.h>
#include <qlabel.h>

#include "jabberprotocol.h"

dlgJabberChatRoomsList::dlgJabberChatRoomsList(JabberAccount* account, const QString& server, const QString &nick, QWidget *parent) 
: KDialog(parent), m_account(account) , m_selectedItem(0) ,  m_nick(nick)
{
	setCaption(i18n("List Chatrooms"));
	setButtons( KDialog::User1 | KDialog::Close );
	setButtonGuiItem( KDialog::User1, KGuiItem( i18n("Join") ) );

	QWidget *mainWidget = new QWidget(this);
	m_ui.setupUi(mainWidget);
	setMainWidget(mainWidget);

	if (!server.isNull())
		m_ui.leServer->setText(server);
	else if(m_account->isConnected())
		m_ui.leServer->setText(m_account->server());

	m_chatServer = m_ui.leServer->text();

	if (!server.isNull())
		slotQuery();

	connect(this, SIGNAL(user1Clicked()), this, SLOT(slotJoin()));
	connect(m_ui.pbQuery, SIGNAL(clicked()), this, SLOT(slotQuery()));
	connect(m_ui.tblChatRoomsList, SIGNAL(itemClicked(QTableWidgetItem*)), this, SLOT(slotClick(QTableWidgetItem*)));
	connect(m_ui.tblChatRoomsList, SIGNAL(itemDoubleClicked(QTableWidgetItem*)), this, SLOT(slotDoubleClick(QTableWidgetItem*)));
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

	if( m_selectedItem )
	{
		kDebug (JABBER_DEBUG_GLOBAL) << "join chat room : " <<  m_account->client()->client()->user() << " @ " << m_selectedItem->text() << " on " << m_chatServer;
		m_account->client()->joinGroupChat(m_chatServer, m_selectedItem->text(), m_nick);
	}
}

void dlgJabberChatRoomsList::slotQuery()
{
	if(!m_account->isConnected())
	{
		m_account->errorConnectFirst();
		return;
	}

	m_ui.tblChatRoomsList->clear();

	XMPP::JT_DiscoItems *discoTask = new XMPP::JT_DiscoItems(m_account->client()->rootTask());
	connect (discoTask, SIGNAL(finished()), this, SLOT(slotQueryFinished()));

	m_chatServer = m_ui.leServer->text();
	discoTask->get(m_ui.leServer->text());
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
	m_ui.tblChatRoomsList->setRowCount(items.count());

	int row = 0;
	for (XMPP::DiscoList::const_iterator it = items.begin(); it != items.end(); ++it)
	{
		QTableWidgetItem *row0 = new QTableWidgetItem( (*it).jid().node() );
		QTableWidgetItem *row1 = new QTableWidgetItem( (*it).name() );
		m_ui.tblChatRoomsList->setItem(row, 0, row0);
		m_ui.tblChatRoomsList->setItem(row, 1, row1);
		++row;
	}
}

void dlgJabberChatRoomsList::slotDoubleClick(QTableWidgetItem *item)
{
	m_selectedItem = item;
	slotJoin();
}

void dlgJabberChatRoomsList::slotClick(QTableWidgetItem *item)
{
	m_selectedItem = item;
}

#include "dlgjabberchatroomslist.moc"

