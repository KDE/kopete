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

#include <qinputdialog.h>
#include <qlayout.h>
#include <qtabwidget.h>
#include <qvbox.h>
#include <qstringlist.h>
#include <qsocket.h>

#include <kdebug.h>
#include <ktextbrowser.h>
#include <ktoolbar.h>
#include <kconfig.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kpopupmenu.h>
#include <kfiledialog.h>
#include <kstddirs.h>

#include "kirc.h"
#include "dccconfirm.h"
#include "ircmessage.h"
#include "ircservercontact.h"
#include "irccmdparser.h"
#include "irccontact.h"
#include "ircservermanager.h"
#include "ircmessage.h"
#include "ircdccsend.h"
#include "ircdccview.h"
#include "ircdccreceive.h"
#include "ircconsoleview.h"
#include "messagetransport.h"
#include "kopetemetacontact.h"
#include "kopetecontactlist.h"

IRCServerContact::IRCServerContact(const QString &server, const QString &nickname, bool connectNow, IRCProtocol *protocol)
{
	mProtocol = protocol;
	mManager = mProtocol->manager;
	mNickname = nickname;
	mServer = server;

	init();

	mConsoleView->show();

	if (connectNow == true)
	{
		mWindow->show();
		slotConnectNow();
	}
}

IRCServerContact::IRCServerContact(IRCProtocol *protocol, bool connectNow)
{
	mProtocol = protocol;
	mManager = mProtocol->manager;
	KGlobal::config()->setGroup("IRC");
	mNickname = KGlobal::config()->readEntry("Nickname", "KopeteUser");
	QString tmpServer = KGlobal::config()->readEntry("Server", "");
	if (tmpServer.isEmpty())
	{
		mServer = "(Console)";
	} else {
		mServer = tmpServer;
	}

	init();

	if (connectNow == true)
	{
		mWindow->show();
		mConsoleView->messageBox->setFocus();
	} else {
		mWindow->hide();
	}
}

void IRCServerContact::init()
{
	parser = new IRCCmdParser(mProtocol, this);
	messenger = new IRCMessage();
	tryingQuit = false;
	closing = false;
	engine = new KIRC();
	engine->setVersionString("Kopete IRC 1.0");
// 	engine->setUserString(""); Pull this from a KConfig entry which is set in the preferences!
	engine->setSourceString("Kopete IRC Plugin 1.0 http://kopete.kde.org");
	mQuitMessage = "Using Kopete IRC Plugin";
	QObject::connect(engine, SIGNAL(incomingFailedNickOnLogin(const QString &)), this, SLOT(nickInUseOnLogin(const QString &)));
	QObject::connect(engine, SIGNAL(successfullyChangedNick(const QString &, const QString &)), this, SLOT(slotChangedNick(const QString &, const QString &)));
	QObject::connect(engine, SIGNAL(successfulQuit()), this, SLOT(slotServerHasQuit()));
	QObject::connect(engine, SIGNAL(incomingPrivMessage(const QString &, const QString &, const QString &)), this, SLOT(incomingPrivMessage(const QString &, const QString &, const QString &)));
	QObject::connect(engine, SIGNAL(incomingPrivAction(const QString &, const QString &, const QString &)), this, SLOT(incomingPrivAction(const QString &, const QString &, const QString &)));
	QObject::connect(engine, SIGNAL(incomingDccChatRequest(const QHostAddress &, unsigned int, const QString &, DCCClient &)), this, SLOT(incomingDccChatRequest(const QHostAddress &, unsigned int, const QString &, DCCClient &)));
	QObject::connect(engine, SIGNAL(incomingDccSendRequest(const QHostAddress &, unsigned int, const QString &, const QString &, unsigned int, DCCClient &)), this, SLOT(incomingDccSendRequest(const QHostAddress &, unsigned int, const QString &, const QString &, unsigned int, DCCClient &)));
	mWindow = new IRCChatWindow(mServer, this);
	QObject::connect(mWindow, SIGNAL(windowClosing()), this, SLOT(slotQuitServer()));
	mWindow->mToolBar->insertButton("connect_no", 1, SIGNAL(clicked()), this, SLOT(slotConnectNow()));

	mTabView = new QVBox(mWindow->mTabWidget);
	mConsoleView = new IRCConsoleView(mServer, engine, this, mTabView);
	mWindow->mTabWidget->addTab(mTabView, SmallIconSet("irc_servermsg"),mServer);

	QObject::connect(mConsoleView, SIGNAL(quitRequested()), this, SLOT(slotQuitServer()));
	QObject::connect(engine, SIGNAL(connectedToServer()), this, SLOT(updateToolbar()));
}

void IRCServerContact::slotConnectNow()
{
	if (mWindow == 0)
	{
		mWindow = new IRCChatWindow(mServer, this);
		QObject::connect(mWindow, SIGNAL(windowClosing()), this, SLOT(slotQuitServer()));
		mWindow->mToolBar->insertButton("connect_no", 1, SIGNAL(clicked()), this, SLOT(slotConnectNow()));

		mTabView = new QVBox(mWindow->mTabWidget);
		mConsoleView = new IRCConsoleView(mServer, engine, this, mTabView);
		mWindow->mTabWidget->addTab(mTabView, SmallIconSet("irc_servermsg"),mServer);

		mConsoleView->messageBox->setFocus();
		QObject::connect(mConsoleView, SIGNAL(quitRequested()), this, SLOT(slotQuitServer()));
		QObject::connect(engine, SIGNAL(connectedToServer()), this, SLOT(updateToolbar()));
	}
	if (mServer == "(Console)")
	{
		QString message = i18n("You need to specify a server before trying to connect. The syntax is: /server irc.yourserver.com");
		messenger->displayMessage(MessageTransport(message, QString(""), QString(""), QString(""), engine->nickName(), IRCMessage::UnknownMsg, mServer, mConsoleView->chatView));
		mConsoleView->messageBox->setText("");
		return;
	}
	mWindow->mToolBar->removeItem(1);
	mWindow->mToolBar->insertButton("connect_creating", 1, SIGNAL(clicked()), this, SLOT(slotDisconnectNow()));
	if (engine->isLoggedIn() == false && engine->state() == QSocket::Idle)
	{
		engine->connectToServer(mServer, 6667, QString("kopeteuser"), mNickname);
	} else {
		engine->close();
		slotQuitServer();
		engine->connectToServer(mServer, 6667, QString("kopeteuser"), mNickname);
	}
}

void IRCServerContact::slotDisconnectNow()
{
	mWindow->mToolBar->removeItem(1);
	mWindow->mToolBar->insertButton("stop", 1, SIGNAL(clicked()), this, SLOT(forceDisconnect()));
	tryingQuit = false;
	if (engine->isLoggedIn() == true)
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
			newFile = KFileDialog::getOpenFileName(QString::null, "*.*", mWindow);
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
		QVBox *parent = new QVBox(mWindow->mTabWidget);
		if (type == DCCServer::Chat)
		{
			IRCDCCView *dccView = new IRCDCCView(nickname, this, parent, dccServer);
		} else if (type == DCCServer::File)
		{
			IRCDCCSend *dccView = new IRCDCCSend(nickname, filename, this, parent, dccServer);
		}
		mWindow->mTabWidget->addTab(parent, SmallIconSet("irc_dcc"),nickname);
		mWindow->mTabWidget->showPage(parent);
	} else {
		delete dccServer;
	}
}

void IRCServerContact::incomingDccChatRequest(const QHostAddress &, unsigned int port, const QString &nickname, DCCClient &chatObject)
{
	if (mWindow != 0)
	{
		if (DCCConfirm::confirmRequest(DCCConfirm::Chat, nickname, QString(""), 0, mWindow))
		{
			QVBox *parent = new QVBox(mWindow->mTabWidget);
			IRCDCCView *dccView = new IRCDCCView(nickname, this, parent, &chatObject);
			mWindow->mTabWidget->addTab(parent, SmallIconSet("irc_dcc"),nickname);
			chatObject.dccAccept();
			mWindow->mTabWidget->showPage(parent);
		} else {
			chatObject.dccCancel();
		}
	}
}

void IRCServerContact::incomingDccSendRequest(const QHostAddress &, unsigned int port, const QString &nickname, const QString &filename, unsigned int size, DCCClient &chatObject)
{
	if (mWindow != 0)
	{
		if (DCCConfirm::confirmRequest(DCCConfirm::Send, nickname, filename, size, mWindow))
		{
			QString newFile = KFileDialog::getSaveFileName(filename, "*.*", mWindow);
			if (newFile.isEmpty())
			{
				return;
			}
			QVBox *parent = new QVBox(mWindow->mTabWidget);
			IRCDCCReceive *dccView = new IRCDCCReceive(nickname, newFile, this, parent, &chatObject);
			mWindow->mTabWidget->addTab(parent, SmallIconSet("irc_dcc"),nickname);
			chatObject.dccAccept(newFile);
			mWindow->mTabWidget->showPage(parent);
		} else {
			chatObject.dccCancel();
		}
	}
}

void IRCServerContact::incomingPrivMessage(const QString &originating, const QString &target, const QString &message)
{
	QString queryName = originating.section('!', 0, 0);
	if (queryName.lower() == engine->nickName().lower())
	{
		return;
	}

	if (activeContacts.find(queryName.lower()) == activeContacts.end())
	{
		KopeteMetaContact *m = KopeteContactList::contactList()->findContact(queryName);
		QString protocolID=mProtocol->id();
		(void)new IRCContact(mServer, queryName, 6667, true, this, m, protocolID);
	}
}

void IRCServerContact::incomingPrivAction(const QString &originating, const QString &target, const QString &message)
{
	QString queryName = originating.section('!', 0, 0);
	if (queryName.lower() == engine->nickName().lower())
	{
		return;
	}

	if (activeContacts.find(queryName.lower()) == activeContacts.end())
	{
		KopeteMetaContact *m = KopeteContactList::contactList()->findContact(queryName);
		QString protocolID=mProtocol->id();
		(void)new IRCContact(mServer, queryName, 6667, true, this, m, protocolID);
	}
}

void IRCServerContact::newNickname(const QString &newNick)
{
	mNickname = newNick;
}

void IRCServerContact::slotChangedNick(const QString &, const QString &newNick)
{
	newNickname(newNick);
}

void IRCServerContact::nickInUseOnLogin(const QString &oldNickname)
{
	bool okay = false;
	QString message = i18n("<qt>The nickname ");
	message.append(oldNickname);
	message.append(i18n(" is currently in use by another user. Enter a new nickname you would like to use:"));
	QString title = i18n(oldNickname);
	title.append(i18n(" is currently in use, choose another</qt>"));
	QString suggested = oldNickname;
	suggested.append("-");
	QString newNick = QInputDialog::getText(title, message, QLineEdit::Normal, suggested, &okay);
	if (okay == true && newNick.isEmpty() == false)
	{
		QString title = newNick;
		title.append("@");
		title.append(mServer);
		mManager->linkServer(QString("%1@%2").arg(mNickname).arg(mServer), title);
		engine->changeNickname(newNick);
		newNickname(newNick);
	} else {
		engine->close();
	}
}

void IRCServerContact::forceDisconnect()
{
	tryingQuit = true;
	engine->close();
	slotQuitServer();
	if (mWindow) { //slotQuitServer may delete mWindow!
		mWindow->mToolBar->removeItem(1);
		mWindow->mToolBar->insertButton("connect_no", 1, SIGNAL(clicked()), this, SLOT(slotConnectNow()));
	}
}

void IRCServerContact::slotQuitServer()
{
	if (tryingQuit == false)
	{
		tryingQuit = true;
		engine->quitIRC(mQuitMessage);
	} else {
		mManager->removeServer(QString("%1@%2").arg(mNickname).arg(mServer));
		if (closing == false)
		{
			mWindow->mToolBar->removeItem(1);
			mWindow->mToolBar->insertButton("connect_no", 1, SIGNAL(clicked()), this, SLOT(slotConnectNow()));
		} else {
			if (mWindow != 0)
			{
				delete mWindow;
				mWindow = 0L;
			}
			engine->close();
		}
		tryingQuit = false;
		closing = false;
	}
}

void IRCServerContact::slotServerHasQuit()
{
	tryingQuit = true;
	slotQuitServer();
}

void IRCServerContact::updateToolbar()
{
	mWindow->mToolBar->removeItem(1);
	mWindow->mToolBar->insertButton("connect_established", 1, SIGNAL(clicked()), this, SLOT(slotDisconnectNow()));
}

bool IRCServerContact::parentClosing()
{
	if (engine->isLoggedIn() == true && engine->state() == QSocket::Connected)
	{
		if (KMessageBox::questionYesNo(mWindow, i18n("You are currently connected to the IRC server, are you sure you want to quit now?"), i18n("Are You Sure?"), KStdGuiItem::yes(), KStdGuiItem::no(), "IRCServerQuitAsk") == KMessageBox::Yes)
		{
			closing = true;
			slotQuitServer();
			// We do this here because we want to wait for the server to disconnect first, then later we destroy this class
			return false;
		} else {
			return false;
		}
	} else {
		if (engine->state() != QSocket::Idle)
		{
			if (KMessageBox::questionYesNo(mWindow, i18n("You are currently connecting to an IRC server, are you sure you want to abort connecting and close this window?"), i18n("Are You Sure?"), KStdGuiItem::yes(), KStdGuiItem::no(), "IRCServerQuitAskNotOnline") == KMessageBox::Yes)
			{
				forceDisconnect();
				mManager->removeServer(QString("%1@%2").arg(mNickname).arg(mServer));
			} else {
				return false;
			}
		} else {
			mManager->removeServer(QString("%1@%2").arg(mNickname).arg(mServer));
			closing = true;
			slotQuitServer();
		}
	}
	return true;
}
QString IRCServerContact::id() const
{
	return mServer+mNickname; //FIXME Is this the righway(TM)
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

