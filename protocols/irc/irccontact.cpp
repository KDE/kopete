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



#include "irccontact.h"

#include "irc_channel_tabwidget.h"
#include "ircchatview.h"
#include "ircchatwindow.h"
#include "ircprotocol.h"
#include "ircqueryview.h"
#include "ircservercontact.h"
#include "kirc.h"
#include "kopete.h"
#include "kopetecontactlist.h"
#include "tabcompleter.h"

#include <kaction.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <ksimpleconfig.h>

#include <qiconset.h>
#include <qlayout.h>
#include <qtextedit.h>
#include <qvbox.h>



IRCContact::IRCContact(const QString &server, const QString &target, unsigned int port, bool joinOnConnect, IRCServerContact *contact, KopeteMetaContact *parent, QString &protocolID)
	: KopeteContact(protocolID, parent),
	  m_pActionCollection(new KActionCollection(this, "IRCActionCollection"))
{
	m_engine = contact->engine();
	m_requestedQuit = false;
	added = false;
	contactOnList = false;
	KGlobal::config()->setGroup("IRC");
	QString newServer;

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

IRCContact::IRCContact(const QString &server, const QString &target, unsigned int port, bool joinOnConnect,
		       IRCServerContact *contact, const QStringList /*pendingMessage*/,
		       KopeteMetaContact *parent, QString &protocolID)
	: KopeteContact(protocolID, parent),
	  m_pActionCollection(new KActionCollection(this, "IRCActionCollection"))
{
	m_engine = contact->engine();
	m_requestedQuit = false;
	added = false;
	contactOnList = false;
	KGlobal::config()->setGroup("IRC");
	QString newServer;

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
	: KopeteContact(protocolID, parent),
	  m_pActionCollection(new KActionCollection(this, "IRCActionCollection"))
{
	m_engine = contact->engine();
	m_requestedQuit = false;
	added = false;
	contactOnList = true;
	KGlobal::config()->setGroup("IRC");
	QString newServer;

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

	if (!init())
	{
		delete this;
		return;
	}

	// FIXME: addContact no longer allows adding KopeteContacts. Convert this
	// to use meta contacts!
	//kopeteapp->contactList()->addContact(this, groupName);
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

bool IRCContact::isChannel() const
{
	static QString const sChannelChars(QString::fromLatin1("#!&+"));

	return ((m_targetName.isEmpty() == false)
		&& (sChannelChars.contains(m_targetName[0]) > 0));
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

void IRCContact::incomingPrivMessage(const QString &/*originating*/, const QString &target,
				     const QString &/*message*/)
{
	if (m_targetName.lower() == target.lower())
	{
		if (mTabPage == 0)
		{
			joinNow();
		}
	}
}

void IRCContact::incomingPrivAction(const QString &/*originating*/, const QString &target,
				    const QString &/*message*/)
{
	if (m_targetName.lower() == target.lower())
	{
		if (mTabPage == 0)
		{
			joinNow();
		}
	}
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

KActionCollection* IRCContact::customContextMenuActions()
{
	m_pActionCollection->clear();

	if (isChannel())
	{
		if (mTabPage != 0)
		{
			new KAction(i18n("Part"), KShortcut(),
				    this, SLOT(slotPart()),
				    m_pActionCollection, "part");
		}
// TODO:	popup->insertItem("Hop (Part and Re-join)", this, SLOT(slotHop()));
	}
	else
	{
		if (mTabPage != 0)
		{
			new KAction(i18n("Close"), KShortcut(),
				    this, SLOT(unloading()),
				    m_pActionCollection, "close");
		}
	}
	if (mTabPage == 0)
	{
		if (m_serverContact->engine()->isLoggedIn())
		{
			QString const sLabel(isChannel()
					     ? i18n("Join")
					     : i18n("Open"));
			new KAction(sLabel, KShortcut(),
				    this, SLOT(slotOpen()),
				    m_pActionCollection, "open");
		}
		else
		{
			QString const sLabel(isChannel()
					     ? i18n("Connect && Join")
					     : i18n("Connect && Open"));
			new KAction(sLabel, KShortcut(),
				    this, SLOT(slotOpen()),
				    m_pActionCollection, "open_connect");
		}
	}

	return m_pActionCollection;
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
		chatView->messageEdit()->setFocus();
		return;
	}
	if (queryView !=0)
	{
		queryView->messageEdit()->setFocus();
		return;
	}
}

void IRCContact::slotDeleteContact()
{
	if (KMessageBox::warningYesNo(qApp->mainWidget(),
				      i18n("<qt>Are you sure you want to remove %1 from your contact list?</qt>").arg(displayName()),
				      i18n("Confirmation")) == KMessageBox::Yes)
	{
		if (isChannel())
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
		}
		delete this;
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
		if (isChannel())
		{
			if (chatView !=0)
			{
				chatView->messageView()->append(partWarning);
				chatView->messageView()->scrollToBottom();
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

void IRCContact::slotUserKicked(const QString &user, const QString &channel,
				const QString &/*by*/, const QString &/*reason*/)
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
	if (isChannel())
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
	if (isChannel())
	{
		m_serverContact->chatWindow()->mTabWidget->showPage(mTabPage);
	} else {
		if (!minimize)
		{
			m_serverContact->chatWindow()->mTabWidget->showPage(mTabPage);
		}
	}
}

QString IRCContact::id() const
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

