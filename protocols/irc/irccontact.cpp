/***************************************************************************
                          irccontact.cpp  -  description
                             -------------------
    begin                : Wed Mar 6 2002
    copyright            : (C) 2002 by nbetcher
    email                : nbetcher@usinternet.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qlayout.h>
#include <qtabwidget.h>
#include <qvbox.h>
#include <qiconset.h>
#include <qvbox.h>
#include <qstringlist.h>

#include <klocale.h>
#include <kmessagebox.h>
#include <kdialog.h>
#include <kdebug.h>
#include <kconfig.h>
#include <ktabctl.h>
#include <kstddirs.h>
#include <klocale.h>
#include <kglobal.h>
#include <ksimpleconfig.h>
#include <ktextbrowser.h>
#include <kiconloader.h>

#include <ircchatwindow.h>

#include "kopetestdaction.h"
#include "ircqueryview.h"
#include "irccontact.h"
#include "ircservercontact.h"
#include "kopetecontactlist.h"
#include "kopetewindow.h"


IRCContact::IRCContact(const QString &server, const QString &target, unsigned int port, bool joinOnConnect, IRCServerContact *contact, KopeteMetaContact *parent, QString &protocolID)
	: KopeteContact(protocolID, parent)
{
	m_engine = contact->engine();
	m_requestedQuit = false;
	added = false;
	contactOnList = false;
	KGlobal::config()->setGroup("IRC");
	QString newServer;

	initActions();

	if (server.isEmpty() == true)
	{
		newServer = KGlobal::config()->readEntry("Server", "irc.openprojects.net");
		m_serverName = newServer;
	} else {
		m_serverName = server;
	}
	if (port == 0)
	{
		port = KGlobal::config()->readEntry("Port", "6667").toUInt();
	}
	QString user = "kopeteuser";
	QString nick = KGlobal::config()->readEntry("Nickname", "KopeteUser");

	m_serverContact = contact;
	m_targetName = target;
	m_port = port;
	m_username = user;
	m_nickname = nick;
	mJoinOnConnect = joinOnConnect;

	// Just to be safe!
	mTabPage = 0L;
	queryView = 0L;
	chatView = 0L;

	if (!init())
	{
		delete this;
		return;
	}

	connect(m_serverContact->engine(), SIGNAL(connectionClosed()), this, SLOT(unloading()));

	if (joinOnConnect == true)
	{
		if (m_serverContact->engine()->isLoggedIn() == true)
		{
			joinNow();
		} else {
			QObject::connect(m_serverContact->engine(), SIGNAL(connectedToServer()), this, SLOT(joinNow()));
		}
	}

}

IRCContact::IRCContact(const QString &server, const QString &target, unsigned int port, bool joinOnConnect, IRCServerContact *contact, const QStringList pendingMessage, KopeteMetaContact *parent, QString &protocolID)
	: KopeteContact(protocolID, parent)
{
	m_engine = contact->engine();
	m_requestedQuit = false;
	added = false;
	contactOnList = false;
	KGlobal::config()->setGroup("IRC");
	QString newServer;

	initActions();

	if (server.isEmpty() == true)
	{
		newServer = KGlobal::config()->readEntry("Server", "");
		m_serverName = newServer;
	} else {
		m_serverName = server;
	}
	if (port == 0)
	{
		port = KGlobal::config()->readEntry("Port", "6667").toUInt();
	}
	QString user = "kopeteuser";
	QString nick = KGlobal::config()->readEntry("Nickname", "KopeteUser");

	m_serverContact = contact;
	m_targetName = target;
	m_port = port;
	m_username = user;
	m_nickname = nick;
	mJoinOnConnect = joinOnConnect;

	// Just to be safe!
	mTabPage = 0L;
	queryView = 0L;
	chatView = 0L;

	if (!init())
	{
		delete this;
		return;
	}

	connect(m_serverContact->engine(), SIGNAL(connectionClosed()), this, SLOT(unloading()));

	if (joinOnConnect == true)
	{
		if (m_serverContact->engine()->isLoggedIn() == true)
		{
			joinNow();
		} else {
			QObject::connect(m_serverContact->engine(), SIGNAL(connectedToServer()), this, SLOT(joinNow()));
		}
	}
}

IRCContact::IRCContact(const QString &groupName, const QString &server, const QString &target, unsigned int port, bool joinOnConnect, IRCServerContact *contact, KopeteMetaContact *parent, QString &protocolID)
	: KopeteContact(protocolID, parent)
{
	m_engine = contact->engine();
	m_requestedQuit = false;
	added = false;
	contactOnList = true;
	KGlobal::config()->setGroup("IRC");
	QString newServer;

	initActions();

	if (server.isEmpty() == true)
	{
		newServer = KGlobal::config()->readEntry("Server", "");
		m_serverName = newServer;
	} else {
		m_serverName = server;
	}
	if (port == 0)
	{
		port = KGlobal::config()->readEntry("Port", "6667").toUInt();
	}
	QString user = "kopeteuser";
	QString nick = KGlobal::config()->readEntry("Nickname", "KopeteUser");

	m_serverContact = contact;
	m_targetName = target;
	m_port = port;
	m_username = user;
	m_nickname = nick;
	mJoinOnConnect = joinOnConnect;
	m_groupName = groupName;

	// Just to be safe!
	mTabPage = 0L;
	queryView = 0L;
	chatView = 0L;

	connect(KopeteContactList::contactList(), SIGNAL(groupRemoved(const QString &)), this, SLOT(slotGroupRemoved(const QString &)));

	if (!init())
	{
		delete this;
		return;
	}

	kopeteapp->contactList()->addContact(this, groupName);
	m_serverContact->protocol()->config()->setGroup(m_targetName.lower());
	m_serverContact->protocol()->config()->writeEntry("Server", m_serverName);
	m_serverContact->protocol()->config()->writeEntry("Group", groupName);
	m_serverContact->protocol()->config()->sync();

	setDisplayName(QString("%1@%2").arg(target).arg(m_serverName));

	connect(m_serverContact->engine(), SIGNAL(connectionClosed()), this, SLOT(unloading()));

	if (joinOnConnect == true)
	{
		if (m_serverContact->engine()->isLoggedIn() == true)
		{
			joinNow();
		} else {
			QObject::connect(m_serverContact->engine(), SIGNAL(connectedToServer()), this, SLOT(joinNow()));
		}
	}
}

void IRCContact::slotGroupRemoved(const QString &group)
{
	if (group == m_groupName)
	{
		slotRemoveThis();
	}
}

bool IRCContact::init()
{
	if (m_serverContact->activeContacts().contains(m_targetName.lower()) > 0)
	{
		return false;
	}
	m_serverContact->activeContacts().append(m_targetName.lower());
	added = true;
	connect(m_serverContact->engine(), SIGNAL(successfulQuit()), this, SLOT(unloading()));
	connect(m_serverContact->engine(), SIGNAL(incomingPartedChannel(const QString &, const QString &, const QString &)), this, SLOT(slotPartedChannel(const QString &, const QString &, const QString &)));
	connect(m_serverContact->engine(), SIGNAL(incomingKick(const QString &, const QString &, const QString &, const QString &)), this, SLOT(slotUserKicked(const QString &, const QString &, const QString &, const QString &)));
	connect(m_engine, SIGNAL(incomingPrivMessage(const QString &, const QString &, const QString &)), this, SLOT(incomingPrivMessage(const QString &, const QString &, const QString &)));
	connect(m_engine, SIGNAL(incomingPrivAction(const QString &, const QString &, const QString &)), this, SLOT(incomingPrivAction(const QString &, const QString &, const QString &)));
	return true;
}

KopeteContact::ContactStatus IRCContact::status() const
{
	if (m_serverContact->engine()->isLoggedIn())
	{
		return KopeteContact::Online;
	}
	return KopeteContact::Offline;
}

QString IRCContact::statusIcon() const
{
	if (m_serverContact->engine()->isLoggedIn())
	{
		return "connect_established";
	} else {
		return "connect_no";
	}
}

void IRCContact::incomingPrivMessage(const QString &originating, const QString &target, const QString &message)
{
	if (m_targetName.lower() == target.lower())
	{
		if (mTabPage == 0)
		{
			joinNow();
		}
	}
}

void IRCContact::incomingPrivAction(const QString &originating, const QString &target, const QString &message)
{
	if (m_targetName.lower() == target.lower())
	{
		if (mTabPage == 0)
		{
			joinNow();
		}
	}
}

void IRCContact::slotRemoveThis()
{
	if (m_targetName[0] == '#' || m_targetName[0] == '!' || m_targetName[0] == '&')
	{
		slotPart();
	}
	m_serverContact->protocol()->config()->deleteGroup(m_targetName.lower());
	m_serverContact->protocol()->config()->sync();
	if (mTabPage !=0)
	{
		m_serverContact->chatWindow()->mTabWidget->removePage(mTabPage);
		delete mTabPage;
		mTabPage = 0L;
		queryView = 0L;
		chatView = 0L;
		delete this;
		return;
	}
	delete this;
}

void IRCContact::slotOpen()
{
	if (!m_serverContact->engine()->isLoggedIn())
	{
		slotOpenConnect();
	} else {
		if (m_serverContact->chatWindow() != 0)
		{
			m_serverContact->chatWindow()->show();
		}
		joinNow();
	}
}

void IRCContact::slotOpenConnect()
{
	if (!m_serverContact->engine()->isLoggedIn())
	{
		QObject::disconnect(m_serverContact->engine(), SIGNAL(connectedToServer()), this, SLOT(joinNow()));
		QObject::connect(m_serverContact->engine(), SIGNAL(connectedToServer()), this, SLOT(joinNow()));
		m_serverContact->slotConnectNow();
		m_serverContact->chatWindow()->show();
	} else {
		slotOpen();
	}
}

void IRCContact::showContextMenu(const QPoint& point, const QString& /*group*/)
{
	popup = new KPopupMenu();
	popup->insertTitle(m_targetName);
	if (m_targetName[0] == '#' || m_targetName[0] == '!' || m_targetName[0] == '&')
	{
		if (mTabPage != 0)
		{
			popup->insertItem(i18n("Part"), this, SLOT(slotPart()));
		}
// TODO:	popup->insertItem("Hop (Part and Re-join)", this, SLOT(slotHop()));
// TODO:	popup->insertItem("Remove", this, SLOT(slotRemoveThis()));
	} else {
		if (mTabPage != 0)
		{
			popup->insertItem(i18n("Close"), this, SLOT(unloading()));
		}
	}
	if (mTabPage == 0)
	{
		if (m_serverContact->engine()->isLoggedIn())
		{
			if (m_targetName[0] == '#' || m_targetName[0] == '!' || m_targetName[0] == '&')
			{
				popup->insertItem(i18n("Join"), this, SLOT(slotOpen()));
			} else {
				popup->insertItem(i18n("Open"), this, SLOT(slotOpen()));
			}
		} else {
			if (m_targetName[0] == '#' || m_targetName[0] == '!' || m_targetName[0] == '&')
			{
				popup->insertItem(i18n("Connect && Join"), this, SLOT(slotOpenConnect()));
			} else {
				popup->insertItem(i18n("Connect && Open"), this, SLOT(slotOpenConnect()));
			}
		}
	}

	popup->insertSeparator();

	actionAddGroup->plug( popup );
	actionContactMove->plug( popup );
	actionRemove->plug( popup );

	popup->popup(point);
}

void IRCContact::execute()
{
	if (m_serverContact->chatWindow() != 0)
	{
		m_serverContact->chatWindow()->raise();
		if (mTabPage !=0)
		{
			m_serverContact->chatWindow()->mTabWidget->showPage(mTabPage);
		}
	}
	if (chatView !=0)
	{
		chatView->messageBox->setFocus();
		return;
	}
	if (queryView !=0)
	{
		queryView->messageBox->setFocus();
		return;
	}
}

void IRCContact::slotPart()
{
	if (chatView != 0)
	{
		QColor color(175, 8, 8);
		QString partWarning = "<font color=";
		partWarning.append(color.name());
		partWarning.append(">Attempting to part channel. If this takes an unusual amount of time, please click the close button on this window again, or right click on the contact in the Kopete window and click \"Part\" again.</font><br>");
		if (m_targetName[0] == '#' || m_targetName[0] == '!' || m_targetName[0] == '&')
		{
			if (chatView !=0)
			{
				chatView->chatView->append(partWarning);
				chatView->chatView->scrollToBottom();
			}
			m_waitingPart = true;
			m_serverContact->engine()->partChannel(m_targetName ,QString("Using Kopete IRC Plugin"));
		}
	}
}

void IRCContact::slotPartedChannel(const QString &originating, const QString &channel, const QString &)
{
	if (m_targetName.lower() == channel.lower() && originating.section('!', 0, 0).lower() == m_engine->nickName().lower())
	{
		unloading();
	}
}

void IRCContact::slotUserKicked(const QString &user, const QString &channel, const QString &by, const QString &reason)
{
	if (m_targetName.lower() == channel.lower() && user.lower() == m_engine->nickName().lower())
	{
		unloading();
	}
}

IRCContact::~IRCContact()
{
	if (added && !contactOnList)
	{
		m_serverContact->activeContacts().remove(m_targetName.lower());
	}
}

void IRCContact::unloading()
{
	if (mTabPage !=0)
	{
		if (m_serverContact->closing() == false)
		{
			delete mTabPage;
		}
		mTabPage = 0L;
		chatView = 0L;
		queryView = 0L;
		if (!contactOnList)
		{
			delete this;
		}
	}
}

void IRCContact::joinNow()
{
	kdDebug() << "IRC Plugin: IRCContact::joinNow() creating mTabPage!" << endl;
	mTabPage = new QVBox(m_serverContact->chatWindow()->mTabWidget);
	if (m_targetName[0] == '#' || m_targetName[0] == '!' || m_targetName[0] == '&')
	{
		chatView = new IRCChatView(m_serverName, m_targetName, this, mTabPage);
		m_serverContact->chatWindow()->mTabWidget->addTab(mTabPage, SmallIconSet("irc_privmsg"), m_targetName);
	} else {
		queryView = new IRCQueryView(m_serverName, m_targetName, m_serverContact, mTabPage, this);
		m_serverContact->chatWindow()->mTabWidget->addTab(mTabPage, SmallIconSet("irc_querymsg"), m_targetName);
	}

	m_serverContact->chatWindow()->show();

	KGlobal::config()->setGroup("IRC");
	bool minimize = KGlobal::config()->readBoolEntry("MinimizeNewQueries", false);
	if (m_targetName[0] == '#' || m_targetName[0] == '!' || m_targetName[0] == '&')
	{
		m_serverContact->chatWindow()->mTabWidget->showPage(mTabPage);
	} else {
		if (!minimize)
		{
			m_serverContact->chatWindow()->mTabWidget->showPage(mTabPage);
		}
	}
}

void IRCContact::slotMoveThisUser() {
	QString mGroup = actionContactMove->currentText();

	kopeteapp->contactList()->moveContact(this, mGroup);

	m_serverContact->protocol()->config()->setGroup(m_targetName.lower());
	m_serverContact->protocol()->config()->writeEntry("Group", mGroup);
	m_serverContact->protocol()->config()->sync();
}

void IRCContact::initActions()
{
	actionAddGroup = KopeteStdAction::addGroup( KopeteContactList::contactList(), SLOT(addGroup()), this, "actionAddGroup" );
	actionContactMove = KopeteStdAction::moveContact( this, SLOT(slotMoveThisUser()), this, "actionMove" );
	actionRemove = KopeteStdAction::deleteContact( this, SLOT(slotRemoveThis()), this, "actionDelete" );
}

QString IRCContact::id() const
{
	return m_username+m_nickname; //FIXME Is this the righway(TM)
}

QString IRCContact::data() const
{
	return m_username+m_nickname; //FIXME Is this the righway(TM)
}
#include "irccontact.moc"
/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 8
 * indent-tabs-mode: t
 * End:
 */
// vim: set noet ts=4 sts=4 sw=4:

