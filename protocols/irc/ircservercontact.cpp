/***************************************************************************
                          ircservercontact.cpp  -  description
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



#include "ircservercontact.h"

#include "dccconfirm.h"
#include "irc_channel_tabwidget.h"
#include "ircchatwindow.h"
#include "irccmdparser.h"
#include "ircconsoleview.h"
#include "irccontact.h"
#include "ircdccreceive.h"
#include "ircdccsend.h"
#include "ircdccview.h"
#include "ircmessage.h"
#include "ircprotocol.h"
#include "ircservermanager.h"
#include "kirc.h"
#include "kopetecontactlist.h"
#include "kopetemetacontact.h"
#include "messagetransport.h"
#include "tabcompleter.h"

#include <kconfig.h>
#include <kdebug.h>
#include <kfiledialog.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kpopupmenu.h>
#include <kstandarddirs.h>
#include <ktextbrowser.h>
#include <ktoolbar.h>

#include <qinputdialog.h>
#include <qlayout.h>
#include <qsocket.h>
#include <qvbox.h>



IRCServerContact::IRCServerContact(const QString &server, const QString &nickname, bool connectNow, IRCProtocol *protocol)
{
	m_protocol = protocol;
	m_serverManager = m_protocol->serverManager();
	m_nickname = nickname;
	m_serverName = server;

	init();

	m_consoleView->show();

	if (connectNow == true)
	{
		m_ircChatWindow->show();
		slotConnectNow();
	}
}

IRCServerContact::IRCServerContact(IRCProtocol *protocol, bool connectNow)
{
	m_protocol = protocol;
	m_serverManager = m_protocol->serverManager();
	KGlobal::config()->setGroup("IRC");
	m_nickname = KGlobal::config()->readEntry("Nickname", "KopeteUser");
	QString tmpServer = KGlobal::config()->readEntry("Server", "");

	if (tmpServer.isEmpty())
	{
		m_serverName = "(Console)";
	}
	else
	{
		m_serverName = tmpServer;
	}

	init();

	if (connectNow == true)
	{
		m_ircChatWindow->show();
		m_consoleView->messageEdit()->setFocus();
	}
	else
	{
		m_ircChatWindow->hide();
	}
}

void IRCServerContact::init()
{
	m_parser = new IRCCmdParser(m_protocol, this);
	m_messenger = new IRCMessage();
	m_tryingQuit = false;
	m_closing = false;
	m_engine = new KIRC();
	m_engine->setVersionString("Kopete IRC 1.0");
// 	m_engine->setUserString(""); Pull this from a KConfig entry which is set in the preferences!
	m_engine->setSourceString("Kopete IRC Plugin 1.0 http://kopete.kde.org");
	mQuitMessage = "Using Kopete IRC Plugin";
	QObject::connect(m_engine, SIGNAL(incomingFailedNickOnLogin(const QString &)), this, SLOT(nickInUseOnLogin(const QString &)));
	QObject::connect(m_engine, SIGNAL(successfullyChangedNick(const QString &, const QString &)), this, SLOT(slotChangedNick(const QString &, const QString &)));
	QObject::connect(m_engine, SIGNAL(successfulQuit()), this, SLOT(slotServerHasQuit()));
	QObject::connect(m_engine, SIGNAL(incomingPrivMessage(const QString &, const QString &, const QString &)), this, SLOT(incomingPrivMessage(const QString &, const QString &, const QString &)));
	QObject::connect(m_engine, SIGNAL(incomingPrivAction(const QString &, const QString &, const QString &)), this, SLOT(incomingPrivAction(const QString &, const QString &, const QString &)));
	QObject::connect(m_engine, SIGNAL(incomingDccChatRequest(const QHostAddress &, unsigned int, const QString &, DCCClient &)), this, SLOT(incomingDccChatRequest(const QHostAddress &, unsigned int, const QString &, DCCClient &)));
	QObject::connect(m_engine, SIGNAL(incomingDccSendRequest(const QHostAddress &, unsigned int, const QString &, const QString &, unsigned int, DCCClient &)), this, SLOT(incomingDccSendRequest(const QHostAddress &, unsigned int, const QString &, const QString &, unsigned int, DCCClient &)));
	m_ircChatWindow = new IRCChatWindow(m_serverName, this);
	QObject::connect(m_ircChatWindow, SIGNAL(windowClosing()), this, SLOT(slotQuitServer()));
	m_ircChatWindow->mToolBar->insertButton("connect_no", 1, SIGNAL(clicked()), this, SLOT(slotConnectNow()));

	mTabView = new QVBox(m_ircChatWindow->mTabWidget);
	m_consoleView = new IRCConsoleView(m_serverName, m_engine, this, mTabView);
	m_ircChatWindow->mTabWidget->addTab(mTabView, SmallIconSet("irc_servermsg"),m_serverName);

	QObject::connect(m_consoleView, SIGNAL(quitRequested()), this, SLOT(slotQuitServer()));
	QObject::connect(m_engine, SIGNAL(connectedToServer()), this, SLOT(updateToolbar()));
}

void IRCServerContact::slotConnectNow()
{
	if (m_ircChatWindow == 0)
	{
		m_ircChatWindow = new IRCChatWindow(m_serverName, this);
		QObject::connect(m_ircChatWindow, SIGNAL(windowClosing()), this, SLOT(slotQuitServer()));
		m_ircChatWindow->mToolBar->insertButton("connect_no", 1, SIGNAL(clicked()), this, SLOT(slotConnectNow()));

		mTabView = new QVBox(m_ircChatWindow->mTabWidget);
		m_consoleView = new IRCConsoleView(m_serverName, m_engine, this, mTabView);
		m_ircChatWindow->mTabWidget->addTab(mTabView, SmallIconSet("irc_servermsg"),m_serverName);

		m_consoleView->messageEdit()->setFocus();
		QObject::connect(m_consoleView, SIGNAL(quitRequested()), this, SLOT(slotQuitServer()));
		QObject::connect(m_engine, SIGNAL(connectedToServer()), this, SLOT(updateToolbar()));
	}
	if (m_serverName == "(Console)")
	{
		QString message = i18n("You need to specify a server before trying to connect. The syntax is: /server irc.yourserver.com");
		m_messenger->displayMessage(MessageTransport(message, QString(""), QString(""), QString(""), m_engine->nickName(), IRCMessage::UnknownMsg, m_serverName, m_consoleView->messageView()));
		m_consoleView->messageEdit()->clear();
		return;
	}
	m_ircChatWindow->mToolBar->removeItem(1);
	m_ircChatWindow->mToolBar->insertButton("connect_creating", 1, SIGNAL(clicked()), this, SLOT(slotDisconnectNow()));
	if (m_engine->isLoggedIn() == false && m_engine->state() == QSocket::Idle)
	{
		m_engine->connectToServer(m_serverName, 6667, QString("kopeteuser"), m_nickname);
	} else {
		m_engine->close();
		slotQuitServer();
		m_engine->connectToServer(m_serverName, 6667, QString("kopeteuser"), m_nickname);
	}
}

void IRCServerContact::slotDisconnectNow()
{
	m_ircChatWindow->mToolBar->removeItem(1);
	m_ircChatWindow->mToolBar->insertButton("stop", 1, SIGNAL(clicked()), this, SLOT(forceDisconnect()));
	m_tryingQuit = false;
	if (m_engine->isLoggedIn() == true)
	{
		slotQuitServer();
	} else {
		forceDisconnect();
	}
}

void IRCServerContact::initiateDcc(const QString &nickname, const QString &filename, DCCServer::Type type)
{
	QString newFile = filename;
	if (type == DCCServer::File)
	{
		if (newFile.isEmpty())
		{
			newFile = KFileDialog::getOpenFileName(QString::null, "*.*", m_ircChatWindow);
			if (newFile.isEmpty())
			{
				return;
			}
		}
	}
	DCCServer *dccServer = new DCCServer(type, newFile);
	kdDebug() << "IRC Plugin: dccServer->ok() == " << dccServer->ok() << endl;
	if (dccServer->ok())
	{
		unsigned int port = dccServer->port();
		QVBox *parent = new QVBox(m_ircChatWindow->mTabWidget);
		if (type == DCCServer::Chat)
		{
			IRCDCCView *dccView = new IRCDCCView(nickname, this, parent, dccServer);
		} else if (type == DCCServer::File)
		{
			IRCDCCSend *dccView = new IRCDCCSend(nickname, filename, this, parent, dccServer);
		}
		m_ircChatWindow->mTabWidget->addTab(parent, SmallIconSet("irc_dcc"),nickname);
		m_ircChatWindow->mTabWidget->showPage(parent);
	} else {
		delete dccServer;
	}
}

void IRCServerContact::incomingDccChatRequest(const QHostAddress &, unsigned int port, const QString &nickname, DCCClient &chatObject)
{
	if (m_ircChatWindow != 0)
	{
		if (DCCConfirm::confirmRequest(DCCConfirm::Chat, nickname, QString(""), 0, m_ircChatWindow))
		{
			QVBox *parent = new QVBox(m_ircChatWindow->mTabWidget);
			IRCDCCView *dccView = new IRCDCCView(nickname, this, parent, &chatObject);
			m_ircChatWindow->mTabWidget->addTab(parent, SmallIconSet("irc_dcc"),nickname);
			chatObject.dccAccept();
			m_ircChatWindow->mTabWidget->showPage(parent);
		} else {
			chatObject.dccCancel();
		}
	}
}

void IRCServerContact::incomingDccSendRequest(const QHostAddress &, unsigned int port, const QString &nickname, const QString &filename, unsigned int size, DCCClient &chatObject)
{
	if (m_ircChatWindow != 0)
	{
		if (DCCConfirm::confirmRequest(DCCConfirm::Send, nickname, filename, size, m_ircChatWindow))
		{
			QString newFile = KFileDialog::getSaveFileName(filename, "*.*", m_ircChatWindow);
			if (newFile.isEmpty())
			{
				return;
			}
			QVBox *parent = new QVBox(m_ircChatWindow->mTabWidget);
			IRCDCCReceive *dccView = new IRCDCCReceive(nickname, newFile, this, parent, &chatObject);
			m_ircChatWindow->mTabWidget->addTab(parent, SmallIconSet("irc_dcc"),nickname);
			chatObject.dccAccept(newFile);
			m_ircChatWindow->mTabWidget->showPage(parent);
		} else {
			chatObject.dccCancel();
		}
	}
}

void IRCServerContact::incomingPrivMessage(const QString &originating, const QString &target, const QString &message)
{
	QString queryName = originating.section('!', 0, 0);
	if (queryName.lower() == m_engine->nickName().lower())
	{
		return;
	}

	if (m_activeContacts.find(queryName.lower()) == m_activeContacts.end())
	{
		KopeteMetaContact *m = KopeteContactList::contactList()->findContact(m_protocol->id(), QString::null, queryName);
		//FIXME: make this better
		if(!m)
		{
			m=new KopeteMetaContact();
			KopeteContactList::contactList()->addMetaContact(m);
		}
		QString protocolID=m_protocol->id();
		(void)new IRCContact(m_serverName, queryName, 6667, true, this, m, protocolID);
	}
}

void IRCServerContact::incomingPrivAction(const QString &originating, const QString &target, const QString &message)
{
	QString queryName = originating.section('!', 0, 0);
	if (queryName.lower() == m_engine->nickName().lower())
	{
		return;
	}

	if (m_activeContacts.find(queryName.lower()) == m_activeContacts.end())
	{
		KopeteMetaContact *m = KopeteContactList::contactList()->findContact(m_protocol->id(), QString::null,queryName);
		//FIXME: make this better
		if(!m)
		{
			m=new KopeteMetaContact();
			KopeteContactList::contactList()->addMetaContact(m);
		}
		QString protocolID=m_protocol->id();
		(void)new IRCContact(m_serverName, queryName, 6667, true, this, m, protocolID);
	}
}

void IRCServerContact::newNickname(const QString &newNick)
{
	m_nickname = newNick;
}

void IRCServerContact::slotChangedNick(const QString &, const QString &newNick)
{
	newNickname(newNick);
}

void IRCServerContact::nickInUseOnLogin(const QString &oldNickname)
{
	bool okay = false;
	QString message = i18n("<qt>The nickname %1 is currently in use by another user. Enter a new nickname you would like to use:</qt>").arg(oldNickname);
	QString title = i18n("<qt>%1 is currently in use, choose another</qt>").arg(oldNickname);
	QString suggested = oldNickname;
	suggested.append("-");
	QString newNick = QInputDialog::getText(title, message, QLineEdit::Normal, suggested, &okay);
	if (okay == true && newNick.isEmpty() == false)
	{
		QString title = newNick;
		title.append("@");
		title.append(m_serverName);
		m_serverManager->linkServer(QString("%1@%2").arg(m_nickname).arg(m_serverName), title);
		m_engine->changeNickname(newNick);
		newNickname(newNick);
	} else {
		m_engine->close();
	}
}

void IRCServerContact::forceDisconnect()
{
	m_tryingQuit = true;
	m_engine->close();
	slotQuitServer();
	if (m_ircChatWindow) { //slotQuitServer may delete m_ircChatWindow!
		m_ircChatWindow->mToolBar->removeItem(1);
		m_ircChatWindow->mToolBar->insertButton("connect_no", 1, SIGNAL(clicked()), this, SLOT(slotConnectNow()));
	}
}

void IRCServerContact::slotQuitServer()
{
	if (m_tryingQuit == false)
	{
		m_tryingQuit = true;
		m_engine->quitIRC(mQuitMessage);
	} else {
		m_serverManager->removeServer(QString("%1@%2").arg(m_nickname).arg(m_serverName));
		if (m_closing == false)
		{
			m_ircChatWindow->mToolBar->removeItem(1);
			m_ircChatWindow->mToolBar->insertButton("connect_no", 1, SIGNAL(clicked()), this, SLOT(slotConnectNow()));
		} else {
			if (m_ircChatWindow != 0)
			{
				delete m_ircChatWindow;
				m_ircChatWindow = 0L;
			}
			m_engine->close();
		}
		m_tryingQuit = false;
		m_closing = false;
	}
}

void IRCServerContact::slotServerHasQuit()
{
	m_tryingQuit = true;
	slotQuitServer();
}

void IRCServerContact::updateToolbar()
{
	m_ircChatWindow->mToolBar->removeItem(1);
	m_ircChatWindow->mToolBar->insertButton("connect_established", 1, SIGNAL(clicked()), this, SLOT(slotDisconnectNow()));
}

bool IRCServerContact::parentClosing()
{
	if (m_engine->isLoggedIn() == true && m_engine->state() == QSocket::Connected)
	{
		if (KMessageBox::questionYesNo(m_ircChatWindow, i18n("You are currently connected to the IRC server, are you sure you want to quit now?"), i18n("Are You Sure?"), KStdGuiItem::yes(), KStdGuiItem::no(), "IRCServerQuitAsk") == KMessageBox::Yes)
		{
			m_closing = true;
			slotQuitServer();
			// We do this here because we want to wait for the server to disconnect first, then later we destroy this class
			return false;
		} else {
			return false;
		}
	} else {
		if (m_engine->state() != QSocket::Idle)
		{
			if (KMessageBox::questionYesNo(m_ircChatWindow, i18n("You are currently connecting to an IRC server, are you sure you want to abort connecting and close this window?"), i18n("Are You Sure?"), KStdGuiItem::yes(), KStdGuiItem::no(), "IRCServerQuitAskNotOnline") == KMessageBox::Yes)
			{
				forceDisconnect();
				m_serverManager->removeServer(QString("%1@%2").arg(m_nickname).arg(m_serverName));
			} else {
				return false;
			}
		} else {
			m_serverManager->removeServer(QString("%1@%2").arg(m_nickname).arg(m_serverName));
			m_closing = true;
			slotQuitServer();
		}
	}
	return true;
}
QString IRCServerContact::id() const
{
	return m_serverName+m_nickname; //FIXME Is this the righway(TM)
}

#include "ircservercontact.moc"
/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 8
 * indent-tabs-mode: t
 * End:
 */
// vim: set noet ts=4 sts=4 sw=4:

