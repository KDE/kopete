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
#include <kmessagebox.h>
#include <kdebug.h>
#include <kconfig.h>
#include <kstddirs.h>

IRCContact::IRCContact(QListViewItem *parent, const QString &server, const QString &target, unsigned int port, bool joinOnConnect, IRCServerContact *contact)
	: IMContact(parent)
{
	engine = contact->engine;
	requestedQuit = false;
	KGlobal::config()->setGroup("IRC");
	QString newServer;
	if (server.isEmpty() == true)
	{
		newServer = KGlobal::config()->readEntry("Server", "irc.openprojects.net");
		mServer = newServer;
	} else {
		mServer = server;
	}
	if (port == 0)
	{
		unsigned int port = KGlobal::config()->readEntry("Port", "6667").toUInt();
	}
	QString user = "kopeteuser";
	QString nick = KGlobal::config()->readEntry("Nickname", "KopeteUser");

	mContact = contact;
	mTarget = target;
	mPort = port;
	mUsername = user;
	mNickname = nick;
	mJoinOnConnect = joinOnConnect;

	if (target[0] == '#' || target[0] == '!' || target[0] == '&')
	{
		setPixmap(0, locate("data", "kopete/pics/irc_privmsg.xpm"));
		QString oldTarget = mTarget;
		mTarget = mTarget.remove(0,1);
		setText(0, mTarget);
		mTarget = oldTarget;
	} else {
		setText(0, mTarget);
	}

	connect(mContact, SIGNAL(quittingServer()), this, SLOT(slotServerIsQuitting()));
	connect(mContact, SIGNAL(serverQuit()), this, SLOT(slotServerHasQuit()));
	connect(mContact->engine, SIGNAL(incomingPartedChannel(const QString &, const QString &, const QString &)), this, SLOT(slotPartedChannel(const QString &, const QString &, const QString &)));
	if (joinOnConnect == true)
	{
		if (mContact->engine->isLoggedIn() == true)
		{
			joinNow();
		} else {
			QObject::connect(mContact->engine, SIGNAL(connectedToServer()), this, SLOT(joinNow()));
		}
	}
}

void IRCContact::slotServerHasQuit()
{
	delete this;
}

void IRCContact::slotServerIsQuitting()
{
	if (requestedQuit == false)
	{
		QColor color(175, 8, 8);
		QString partWarning = "<font color=";
		partWarning.append(color.name());
		partWarning.append(">Attempting to quit server. If this takes an unusual amount of time, please right click on one of the channels or server in Kopete contact list and click \"Quit IRC Server\" again.</font><br>");
		chatView->chatView->append(partWarning);
		chatView->chatView->scrollToBottom();
	}
}

void IRCContact::rightButtonPressed(const QPoint &point)
{
	popup = new KPopupMenu();
	popup->insertTitle(mTarget);
	popup->insertItem("Part", this, SLOT(slotPart()));
// TODO:	popup->insertItem("Hop (Part and Re-join)", this, SLOT(slotHop()));
// TODO:	popup->insertItem("Remove", this, SLOT(slotRemoveThis()));
	popup->insertTitle(((QListViewItem *)mContact)->text(0));
	popup->insertItem("Quit IRC Server", this, SLOT(slotQuitServer()));
	popup->popup(point);
}

void IRCContact::slotQuitServer()
{
	requestedQuit = true;
	mContact->slotQuitServer();
}

void IRCContact::leftButtonDoubleClicked()
{
	if (chatView != 0)
	{
		if (chatView->isVisible() == true)
		{
			chatView->raise();
			chatView->setFocus();
			chatView->messageBox->setFocus();
		}
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
		chatView->chatView->append(partWarning);
		chatView->chatView->scrollToBottom();
	}
	waitingPart = true;
	mContact->engine->partChannel(QCString(mTarget.local8Bit()) ,QString("Using Kopete IRC Plugin"));
}

void IRCContact::slotPartedChannel(const QString &originating, const QString &channel, const QString &reason)
{
	if (mTarget.lower() == channel.lower() && originating.left(originating.find("!")).lower() == mContact->mNickname.lower())
	{
		unloading();
	}
}

void IRCContact::unloading()
{
	if (chatView != 0)
	{
		delete chatView;
	}
	mContact->unloading();
	delete this;
}

void IRCContact::slotIncomingMotd(const QString &motd)
{

}

void IRCContact::joinNow()
{
	chatView = new IRCChatView(mServer, mTarget, this);
	chatView->show();
	QObject::connect(mContact->engine, SIGNAL(userJoinedChannel(const QString &, const QString &)), chatView, SLOT(userJoinedChannel(const QString &, const QString &)));
	QObject::connect(mContact->engine, SIGNAL(incomingMessage(const QString &, const QString &, const QString &)), chatView, SLOT(incomingMessage(const QString &, const QString &, const QString &)));
	QObject::connect(mContact->engine, SIGNAL(incomingPartedChannel(const QString &, const QString &, const QString &)), chatView, SLOT(userPartedChannel(const QString &, const QString &, const QString &)));
	QObject::connect(mContact->engine, SIGNAL(incomingNamesList(const QString &, const QString &, const int)), chatView, SLOT(incomingNamesList(const QString &, const QString &, const int)));
	QObject::connect(mContact->engine, SIGNAL(incomingAction(const QString &, const QString &, const QString &)), chatView, SLOT(incomingAction(const QString &, const QString &, const QString &)));
	QObject::connect(mContact->engine, SIGNAL(incomingQuitIRC(const QString &,  const QString &)), chatView, SLOT(userQuitIRC(const QString &, const QString &)));
	QObject::connect(mContact->engine, SIGNAL(incomingNickChange(const QString &,  const QString &)), chatView, SLOT(nickNameChanged(const QString &, const QString &)));
	QObject::connect(mContact->engine, SIGNAL(incomingTopicChange(const QString &, const QString &, const QString &)), chatView, SLOT(incomingNewTopic(const QString &, const QString &, const QString &)));
	QObject::connect(mContact->engine, SIGNAL(incomingExistingTopic(const QString &,  const QString &)), chatView, SLOT(receivedExistingTopic(const QString &, const QString &)));
}
