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
#include <klocale.h>
#include <kdebug.h>
#include <qlayout.h>
#include "ircconsoleview.h"
#include <qtabwidget.h>
#include <qvbox.h>
#include <ktoolbar.h>
#include <ktoolbarbutton.h>
#include <qsocket.h>
#include <kmessagebox.h>
#include "kirc.h"

IRCServerContact::IRCServerContact(const QString &server, const QString &nickname, bool connectNow, IRCServerManager *manager)
	: KListViewItem( kopeteapp->contactList() )
{
	tryingQuit = false;
	completedDisconnect = true;
	engine = new KIRC();
	QObject::connect(engine, SIGNAL(incomingFailedNickOnLogin(const QString &)), this, SLOT(nickInUseOnLogin(const QString &)));
	QObject::connect(engine, SIGNAL(successfullyChangedNick(const QString &, const QString &)), this, SLOT(slotChangedNick(const QString &, const QString &)));
	QObject::connect(engine, SIGNAL(successfulQuit()), this, SLOT(slotServerHasQuit()));
	mManager = manager;
	mNickname = nickname;
	mServer = server;
	mWindow = new IRCChatWindow(mServer, this);
	QObject::connect(mWindow, SIGNAL(windowClosing()), this, SLOT(slotQuitServer()));
	mWindow->mToolBar->insertButton("connect_no", 1, SIGNAL(clicked()), this, SLOT(connectNow()));

	QVBox *tabView = new QVBox(mWindow->mTabWidget);
	IRCConsoleView *consoleView = new IRCConsoleView(mServer, engine, this, tabView);
	mWindow->mTabWidget->addTab(tabView, mServer);

	QObject::connect(consoleView, SIGNAL(quitRequested()), this, SLOT(slotQuitServer()));
	QObject::connect(this, SIGNAL(connecting()), consoleView, SLOT(slotConnecting()));
	QObject::connect(engine, SIGNAL(connectedToServer()), this, SLOT(updateToolbar()));

	mWindow->show();
	consoleView->show();

	QString title = nickname;
	title.append("@");
	title.append(server);
	setText(0, title);
	if (connectNow == true)
	{
		// GCC didn't like me calling connectNow(), not sure why
		mWindow->mToolBar->removeItem(1);
		mWindow->mToolBar->insertButton("connect_creating", 1, SIGNAL(clicked()), this, SLOT(disconnectNow()));
		emit connecting();
		engine->connectToServer(mServer, 6667, QString("kopeteuser"), mNickname);
	}
}

void IRCServerContact::connectNow()
{
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
	completedDisconnect = false;
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

void IRCServerContact::newNickname(const QString &newNick)
{
	mNickname = newNick;
	QString title = newNick;
	title.append("@");
	title.append(mServer);
	setText(0, title);
}

void IRCServerContact::slotChangedNick(const QString &oldNick, const QString &newNick)
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
		mManager->linkServer(text(0), title);
		engine->changeNickname(newNick);
		newNickname(newNick);
	}
}

void IRCServerContact::forceDisconnect()
{
	if (completedDisconnect == false)
	{
		tryingQuit = true;
		emit quittingServer();
		engine->close();
		slotQuitServer();
		completedDisconnect = true;
	}
	mWindow->mToolBar->removeItem(1);
	mWindow->mToolBar->insertButton("connect_no", 1, SIGNAL(clicked()), this, SLOT(connectNow()));
}

void IRCServerContact::slotQuitServer()
{
	if (tryingQuit == false)
	{
		tryingQuit = true;
		emit quittingServer();
		engine->quitIRC("Using Kopete IRC Plugin");
		completedDisconnect = false;
		QTimer::singleShot(10000, this, SLOT(forceDisconnect()));
	} else {
		completedDisconnect = true;
		emit serverQuit();
		mManager->removeServer(text(0));
		mWindow->mToolBar->removeItem(1);
		mWindow->mToolBar->insertButton("connect_no", 1, SIGNAL(clicked()), this, SLOT(connectNow()));
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
			slotQuitServer();
			return true;
		} else {
			return false;
		}
	}
	return true;
}
