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
#include <kdialogbase.h>

IRCServerContact::IRCServerContact(const QString &server, const QString &nickname, bool connectNow, IRCServerManager *manager)
	: KListViewItem( kopeteapp->contactList() )
{
	tryingQuit = false;
	engine = new KIRC();
	QObject::connect(engine, SIGNAL(incomingFailedNickOnLogin(const QString &)), this, SLOT(nickInUseOnLogin(const QString &)));
	QObject::connect(engine, SIGNAL(successfullyChangedNick(const QString &, const QString &)), this, SLOT(slotChangedNick(const QString &, const QString &)));
	QObject::connect(engine, SIGNAL(successfulQuit()), this, SLOT(slotServerHasQuit()));
	mManager = manager;
	mNickname = nickname;
	mServer = server;
	mWindow = new IRCChatWindow(mServer);

	IRCConsoleView *consoleView = new IRCConsoleView(mServer, engine, this, mWindow->addVBoxPage(mServer));

	QObject::connect(consoleView, SIGNAL(quitRequested()), this, SLOT(slotQuitServer()));
	QObject::connect(this, SIGNAL(connecting()), consoleView, SLOT(slotConnecting()));

	mWindow->show();
	consoleView->show();

	QString title = nickname;
	title.append("@");
	title.append(server);
	setText(0, title);
	if (connectNow == true)
	{
		emit connecting();
		engine->connectToServer(server, 6667, QString("kopeteuser"), nickname);
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

void IRCServerContact::slotQuitServer()
{
	if (tryingQuit == false)
	{
		tryingQuit = true;
		emit quittingServer();
		engine->quitIRC("Using Kopete IRC Plugin");
	} else {
		emit serverQuit();
		mManager->removeServer(text(0));
		if (mWindow !=0)
		{
			delete mWindow;
		}
		delete this;
	}
}

void IRCServerContact::slotServerHasQuit()
{
	tryingQuit = true;
	slotQuitServer();
}

void IRCServerContact::unloading()
{
	// It's a hack, what can I say? Send me a patch if it really bothers you.
	QTimer::singleShot(200, this, SLOT(slotPollList()));
}

void IRCServerContact::slotPollList()
{
	if (childCount() == 0)
	{
		tryingQuit = false;
		slotQuitServer();
	}
}
