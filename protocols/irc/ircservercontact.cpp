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
#include <qinputdialog.h>
#include <kconfig.h>
#include <klocale.h>
#include <qinputdialog.h>
#include <kdebug.h>
#include <qlayout.h>
#include "ircconsoleview.h"
#include <qtabwidget.h>
#include <qvbox.h>
#include <ktoolbar.h>
#include <qsocket.h>
#include <kmessagebox.h>
#include <kpopupmenu.h>
#include <qstringlist.h>
#include <kstddirs.h>
#include "kirc.h"
#include "ircmessage.h"
#include "irccmdparser.h"
#include "irccontact.h"
#include "ircservermanager.h"
#include "ircmessage.h"

IRCServerContact::IRCServerContact(const QString &server, const QString &nickname, bool connectNow, IRCProtocol *protocol)
{
	parser = new IRCCmdParser(this);
	messenger = new IRCMessage();
	tryingQuit = false;
	closing = false;
	engine = new KIRC();
	mQuitMessage = "Using Kopete IRC Plugin";
	QObject::connect(engine, SIGNAL(incomingFailedNickOnLogin(const QString &)), this, SLOT(nickInUseOnLogin(const QString &)));
	QObject::connect(engine, SIGNAL(successfullyChangedNick(const QString &, const QString &)), this, SLOT(slotChangedNick(const QString &, const QString &)));
	QObject::connect(engine, SIGNAL(successfulQuit()), this, SLOT(slotServerHasQuit()));
	QObject::connect(engine, SIGNAL(incomingMessage(const QString &, const QString &, const QString &)), this, SLOT(incomingMessage(const QString &, const QString &, const QString &)));
	QObject::connect(engine, SIGNAL(incomingAction(const QString &, const QString &, const QString &)), this, SLOT(incomingAction(const QString &, const QString &, const QString &)));
	mProtocol = protocol;
	mManager = mProtocol->manager;
	mNickname = nickname;
	mServer = server;
	mWindow = new IRCChatWindow(mServer, this);
	QObject::connect(mWindow, SIGNAL(windowClosing()), this, SLOT(slotQuitServer()));
	mWindow->mToolBar->insertButton("connect_no", 1, SIGNAL(clicked()), this, SLOT(connectNow()));

	mTabView = new QVBox(mWindow->mTabWidget);
	mConsoleView = new IRCConsoleView(mServer, engine, this, mTabView);
	mWindow->mTabWidget->addTab(mTabView, mServer);

	QObject::connect(mConsoleView, SIGNAL(quitRequested()), this, SLOT(slotQuitServer()));
	QObject::connect(this, SIGNAL(connecting()), mConsoleView, SLOT(slotConnecting()));
	QObject::connect(engine, SIGNAL(connectedToServer()), this, SLOT(updateToolbar()));

	mWindow->show();
	mConsoleView->show();

	if (connectNow == true)
	{
		// GCC didn't like me calling connectNow(), not sure why
		mWindow->mToolBar->removeItem(1);
		mWindow->mToolBar->insertButton("connect_creating", 1, SIGNAL(clicked()), this, SLOT(disconnectNow()));
		emit connecting();
		engine->connectToServer(mServer, 6667, QString("kopeteuser"), mNickname);
	} else {
		mWindow->hide();
	}
}

IRCServerContact::IRCServerContact(IRCProtocol *protocol, bool connectNow)
{
	mProtocol = protocol;
	parser = new IRCCmdParser(this);
	messenger = new IRCMessage();
	tryingQuit = false;
	closing = false;
	engine = new KIRC();
	mQuitMessage = "Using Kopete IRC Plugin";
	QObject::connect(engine, SIGNAL(incomingFailedNickOnLogin(const QString &)), this, SLOT(nickInUseOnLogin(const QString &)));
	QObject::connect(engine, SIGNAL(successfullyChangedNick(const QString &, const QString &)), this, SLOT(slotChangedNick(const QString &, const QString &)));
	QObject::connect(engine, SIGNAL(successfulQuit()), this, SLOT(slotServerHasQuit()));
	QObject::connect(engine, SIGNAL(incomingMessage(const QString &, const QString &, const QString &)), this, SLOT(incomingMessage(const QString &, const QString &, const QString &)));
	QObject::connect(engine, SIGNAL(incomingAction(const QString &, const QString &, const QString &)), this, SLOT(incomingAction(const QString &, const QString &, const QString &)));
	mManager = mProtocol->manager;
	mNickname = KGlobal::config()->readEntry("Nickname", "KopeteUser");
	mServer = "(Console)";

	mWindow = new IRCChatWindow(mServer, this);
	if (connectNow == false)
	{
		mWindow->hide();
	}
	QObject::connect(mWindow, SIGNAL(windowClosing()), this, SLOT(slotQuitServer()));
	mWindow->mToolBar->insertButton("connect_no", 1, SIGNAL(clicked()), this, SLOT(connectNow()));

	mTabView = new QVBox(mWindow->mTabWidget);
	mConsoleView = new IRCConsoleView(mServer, engine, this, mTabView);
	mWindow->mTabWidget->addTab(mTabView, mServer);

	if (connectNow == true)
	{
		mWindow->show();
		mConsoleView->messageBox->setFocus();
	} else {
		mWindow->hide();
	}

	QObject::connect(mConsoleView, SIGNAL(quitRequested()), this, SLOT(slotQuitServer()));
	QObject::connect(this, SIGNAL(connecting()), mConsoleView, SLOT(slotConnecting()));
	QObject::connect(engine, SIGNAL(connectedToServer()), this, SLOT(updateToolbar()));
}

void IRCServerContact::connectNow()
{
	if (mWindow == 0)
	{
		mWindow = new IRCChatWindow(mServer, this);
		QObject::connect(mWindow, SIGNAL(windowClosing()), this, SLOT(slotQuitServer()));
		mWindow->mToolBar->insertButton("connect_no", 1, SIGNAL(clicked()), this, SLOT(connectNow()));

		mTabView = new QVBox(mWindow->mTabWidget);
		mConsoleView = new IRCConsoleView(mServer, engine, this, mTabView);
		mWindow->mTabWidget->addTab(mTabView, mServer);

		mConsoleView->messageBox->setFocus();
		QObject::connect(mConsoleView, SIGNAL(quitRequested()), this, SLOT(slotQuitServer()));
		QObject::connect(this, SIGNAL(connecting()), mConsoleView, SLOT(slotConnecting()));
		QObject::connect(engine, SIGNAL(connectedToServer()), this, SLOT(updateToolbar()));
	}
	if (mServer == "(Console)")
	{
		QString parsed = QString("<img src=\"%1\"><b>").arg(locate("data", "kopete/pics/irc_unknowncmd.xpm"));
		parsed.append(QStyleSheet::escape(i18n("Sorry, you need to specifiy a server before trying to connect. The syntax is: /server irc.yourserver.com")));
		parsed.append("</b><br>");
		mConsoleView->chatView->append(parsed);
		mConsoleView->chatView->scrollToBottom();
		mConsoleView->messageBox->setText("");
		return;
	}
	mWindow->mToolBar->removeItem(1);
	mWindow->mToolBar->insertButton("connect_creating", 1, SIGNAL(clicked()), this, SLOT(disconnectNow()));
	if (engine->isLoggedIn() == false && engine->state() == QSocket::Idle)
	{
		emit connecting();
		engine->connectToServer(mServer, 6667, QString("kopeteuser"), mNickname);
	} else {
		emit quittingServer();
		engine->close();
		slotQuitServer();
		emit connecting();
		engine->connectToServer(mServer, 6667, QString("kopeteuser"), mNickname);
	}
}

void IRCServerContact::disconnectNow()
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

void IRCServerContact::incomingMessage(const QString &originating, const QString &target, const QString &message)
{
	if (target[0] == '#' || target[0] == '!' || target[0] == '&')
	{
		return;
	}
	QString queryName = originating.section('!', 0, 0);
	if (queryName.lower() == engine->nickName().lower())
	{
		return;
	}
	QStringList pendingMessage;
	pendingMessage << "message" << originating << target << message;
	if (activeQueries.find(queryName.lower()) == activeQueries.end())
	{
		(void)new IRCContact(mServer, queryName, 6667, true, this, pendingMessage);
	} else {
		for (IRCContact *it = activeContacts.first(); it != 0; it = activeContacts.next())
		{
			if (it != 0)
			{
				if (it->mTarget.lower() == queryName.lower())
				{
					if (it->tabPage() == 0)
					{
						it->mPendingMessage = pendingMessage;
						it->joinNow();
					}
					return;
				}
			}
		}
	}
}

void IRCServerContact::incomingAction(const QString &originating, const QString &target, const QString &message)
{
	if (target[0] == '#' || target[0] == '!' || target[0] == '&')
	{
		return;
	}
	QString queryName = originating.section('!', 0, 0);
	if (queryName.lower() == engine->nickName().lower())
	{
		return;
	}
	QStringList pendingMessage;
	pendingMessage << "action" << originating << target << message;
	if (activeQueries.find(queryName.lower()) == activeQueries.end())
	{
		(void)new IRCContact(mServer, queryName, 6667, true, this, pendingMessage);
	} else {
		for (IRCContact *it = activeContacts.first(); it != 0; it = activeContacts.next())
		{
			if (it != 0)
			{
				if (it->mTarget.lower() == queryName.lower())
				{
					if (it->tabPage() == 0)
					{
						it->mPendingMessage = pendingMessage;
						it->joinNow();
					}
					return;
				}
			}
		}
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
	message.append(i18n(" is currently in use by another user. Please enter a new nickname you would like to use:"));
	QString title = i18n(oldNickname);
	title.append(i18n(" is currently in use, chose another</qt>"));
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
	}
}

void IRCServerContact::forceDisconnect()
{
	emit quittingServer();
	engine->close();
	slotQuitServer();
	mWindow->mToolBar->removeItem(1);
	mWindow->mToolBar->insertButton("connect_no", 1, SIGNAL(clicked()), this, SLOT(connectNow()));
}

void IRCServerContact::promptChannelJoin()
{
	bool okay = false;
	QString target = QInputDialog::getText(i18n("Enter the channel to join..."), i18n("<qt>What channel would you like to join?</qt>"), QLineEdit::Normal, QString::null, &okay, mWindow);
	if (okay == true && target.isEmpty() == false)
	{
		if (target[0] == '#' || target[0] == '!' || target[0] == '&')
		{
			(void) new IRCContact(mServer, target, 6667, true, this);
		} else {
			KMessageBox::sorry(mWindow, i18n("<qt>Sorry, you entered an invalid response. Channels are required to begin with a '#', '!', or a '&' by IRC. Please try again.</qt>"), i18n("Invalid Response"));
		}
	}
}

void IRCServerContact::slotQuitServer()
{
	if (tryingQuit == false)
	{
		tryingQuit = true;
		emit quittingServer();
		engine->quitIRC(mQuitMessage);
	} else {
		emit serverQuit();
		mManager->removeServer(QString("%1@%2").arg(mNickname).arg(mServer));
		if (closing == false)
		{
			mWindow->mToolBar->removeItem(1);
			mWindow->mToolBar->insertButton("connect_no", 1, SIGNAL(clicked()), this, SLOT(connectNow()));
		} else {
			if (mWindow != 0)
			{
				delete mWindow;
				mWindow = 0L;
			}
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
	mWindow->mToolBar->insertButton("connect_established", 1, SIGNAL(clicked()), this, SLOT(disconnectNow()));
}

bool IRCServerContact::parentClosing()
{
	if (engine->isLoggedIn() == true || engine->state() != QSocket::Idle)
	{
		if (KMessageBox::questionYesNo(mWindow, i18n("You are currently connected to the IRC server, are you sure you want to quit now?"), i18n("Are you sure?"), KStdGuiItem::yes(), KStdGuiItem::no(), "IRCServerQuitAsk") == KMessageBox::Yes)
		{
			tryingQuit = false;
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
			if (KMessageBox::questionYesNo(mWindow, i18n("You are currently connecting to an IRC server, are you sure you want to abort connecting and close this window?"), i18n("Are you sure?"), KStdGuiItem::yes(), KStdGuiItem::no(), "IRCServerQuitAskNotOnline") == KMessageBox::Yes)
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

#include "ircservercontact.moc"
