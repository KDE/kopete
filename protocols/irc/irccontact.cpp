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

#include <klocale.h>
#include "irccontact.h"
#include "ircservercontact.h"
#include <kmessagebox.h>
#include <qlayout.h>
#include <kdialog.h>
#include <kdebug.h>
#include <kconfig.h>
#include <ktabctl.h>
#include <kstddirs.h>
#include <ircchatwindow.h>
#include <qtabwidget.h>
#include <qvbox.h>
#include <klocale.h>
#include <kglobal.h>
#include "ircqueryview.h"
#include <qiconset.h>
#include <qvbox.h>
#include <qstringlist.h>
#include <ksimpleconfig.h>
#include "kopetestdaction.h"

IRCContact::IRCContact(const QString &server, const QString &target, unsigned int port, bool joinOnConnect, IRCServerContact *contact)
	: KopeteContact(contact->mProtocol)
{
	engine = contact->engine;
	requestedQuit = false;
	KGlobal::config()->setGroup("IRC");
	QString newServer;

	initActions();

	if (server.isEmpty() == true)
	{
		newServer = KGlobal::config()->readEntry("Server", "irc.openprojects.net");
		mServer = newServer;
	} else {
		mServer = server;
	}
	if (port == 0)
	{
		port = KGlobal::config()->readEntry("Port", "6667").toUInt();
	}
	QString user = "kopeteuser";
	QString nick = KGlobal::config()->readEntry("Nickname", "KopeteUser");

	mContact = contact;
	mTarget = target;
	mPort = port;
	mUsername = user;
	mNickname = nick;
	mJoinOnConnect = joinOnConnect;

	// Just to be safe!
	mTabPage = 0L;
	queryView = 0L;
	chatView = 0L;

	mContact->activeContacts.append(this);

	init();

	connect(mContact->engine, SIGNAL(connectionClosed()), this, SLOT(unloading()));

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

IRCContact::IRCContact(const QString &server, const QString &target, unsigned int port, bool joinOnConnect, IRCServerContact *contact, const QStringList pendingMessage)
	: KopeteContact(contact->mProtocol)
{
	engine = contact->engine;
	requestedQuit = false;
	KGlobal::config()->setGroup("IRC");
	QString newServer;

	initActions();

	if (server.isEmpty() == true)
	{
		newServer = KGlobal::config()->readEntry("Server", "");
		mServer = newServer;
	} else {
		mServer = server;
	}
	if (port == 0)
	{
		port = KGlobal::config()->readEntry("Port", "6667").toUInt();
	}
	QString user = "kopeteuser";
	QString nick = KGlobal::config()->readEntry("Nickname", "KopeteUser");

	mContact = contact;
	mTarget = target;
	mPort = port;
	mUsername = user;
	mNickname = nick;
	mJoinOnConnect = joinOnConnect;

	// Just to be safe!
	mTabPage = 0L;
	queryView = 0L;
	chatView = 0L;

	mContact->activeContacts.append(this);

	init();

	connect(mContact->engine, SIGNAL(connectionClosed()), this, SLOT(unloading()));

	mPendingMessage = pendingMessage;

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

IRCContact::IRCContact(const QString &groupName, const QString &server, const QString &target, unsigned int port, bool joinOnConnect, IRCServerContact *contact)
	: KopeteContact(contact->mProtocol)
{
	engine = contact->engine;
	requestedQuit = false;
	KGlobal::config()->setGroup("IRC");
	QString newServer;

	initActions();

	if (server.isEmpty() == true)
	{
		newServer = KGlobal::config()->readEntry("Server", "");
		mServer = newServer;
	} else {
		mServer = server;
	}
	if (port == 0)
	{
		port = KGlobal::config()->readEntry("Port", "6667").toUInt();
	}
	QString user = "kopeteuser";
	QString nick = KGlobal::config()->readEntry("Nickname", "KopeteUser");

	mContact = contact;
	mTarget = target;
	mPort = port;
	mUsername = user;
	mNickname = nick;
	mJoinOnConnect = joinOnConnect;
	mGroupName = groupName;

	// Just to be safe!
	mTabPage = 0L;
	queryView = 0L;
	chatView = 0L;

	mContact->activeContacts.append(this);
	connect(kopeteapp->contactList(), SIGNAL(groupRemoved(const QString &)), this, SLOT(slotGroupRemoved(const QString &)));

	init();

	kopeteapp->contactList()->addContact(this, groupName);
	mContact->mProtocol->mConfig->setGroup(mTarget.lower());
	mContact->mProtocol->mConfig->writeEntry("Server", mServer);
	mContact->mProtocol->mConfig->writeEntry("Group", groupName);
	mContact->mProtocol->mConfig->sync();

	setName(QString("%1@%2").arg(target).arg(mServer));

	connect(mContact->engine, SIGNAL(connectionClosed()), this, SLOT(unloading()));

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

void IRCContact::slotGroupRemoved(const QString &group)
{
	if (group == mGroupName)
	{
		slotRemoveThis();
	}
}

void IRCContact::init()
{
	// Split up into init() so we can call this multiple times and have multiple constructors
	if (mTarget[0] == '#' || mTarget[0] == '!' || mTarget[0] == '&')
	{
		// What to do here, what to do, hmm
	} else {
		// Gosh darn it I love NULL pointer checks ;)
		if (mContact->activeQueries.find(mTarget.lower()) != mContact->activeQueries.end())
		{
			delete this;
			return;
		}
		mContact->activeQueries.append(mTarget.lower());
	}
	connect(mContact->engine, SIGNAL(successfulQuit()), this, SLOT(unloading()));
	connect(mContact->engine, SIGNAL(incomingPartedChannel(const QString &, const QString &, const QString &)), this, SLOT(slotPartedChannel(const QString &, const QString &, const QString &)));
	connect(engine, SIGNAL(incomingPrivMessage(const QString &, const QString &, const QString &)), this, SLOT(incomingPrivMessage(const QString &, const QString &, const QString &)));
	connect(engine, SIGNAL(incomingPrivAction(const QString &, const QString &, const QString &)), this, SLOT(incomingPrivAction(const QString &, const QString &, const QString &)));
}

KopeteContact::ContactStatus IRCContact::status() const
{
	if (mContact->engine->isLoggedIn())
	{
		return KopeteContact::Online;
	}
	return KopeteContact::Offline;
}

QString IRCContact::statusIcon() const
{
	if (mContact->engine->isLoggedIn())
	{
		return "connect_established";
	} else {
		return "connect_no";
	}
}

void IRCContact::incomingPrivMessage(const QString &originating, const QString &target, const QString &message)
{
	if (mTarget.lower() == target.lower())
	{
		if (mTabPage == 0)
		{
			mPendingMessage.clear();
			mPendingMessage << "message" << originating << target << message;
			joinNow();
		}
	}
}

void IRCContact::incomingPrivAction(const QString &originating, const QString &target, const QString &message)
{
	if (mTarget.lower() == target.lower())
	{
		if (mTabPage != 0)
		{
			mPendingMessage.clear();
			mPendingMessage << "action" << originating << target << message;
			joinNow();
		}
	}
}

void IRCContact::slotRemoveThis()
{
	if (mTarget[0] == '#' || mTarget[0] == '!' || mTarget[0] == '&')
	{
		slotPart();
	}
	mContact->mProtocol->mConfig->deleteGroup(mTarget.lower());
	mContact->mProtocol->mConfig->sync();
	if (mTabPage !=0)
	{
		mContact->mWindow->mTabWidget->removePage(mTabPage);
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
	if (!mContact->engine->isLoggedIn())
	{
		slotOpenConnect();
	} else {
		if (mContact->mWindow != 0)
		{
			mContact->mWindow->show();
		}
		joinNow();
	}
}

void IRCContact::slotOpenConnect()
{
	if (!mContact->engine->isLoggedIn())
	{
		QObject::disconnect(mContact->engine, SIGNAL(connectedToServer()), this, SLOT(joinNow()));
		QObject::connect(mContact->engine, SIGNAL(connectedToServer()), this, SLOT(joinNow()));
		mContact->connectNow();
		mContact->mWindow->show();
	} else {
		slotOpen();
	}
}

void IRCContact::showContextMenu(QPoint point, QString /*group*/)
{
	popup = new KPopupMenu();
	popup->insertTitle(mTarget);
	if (mTarget[0] == '#' || mTarget[0] == '!' || mTarget[0] == '&')
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
		if (mContact->engine->isLoggedIn())
		{
			if (mTarget[0] == '#' || mTarget[0] == '!' || mTarget[0] == '&')
			{
				popup->insertItem(i18n("Join"), this, SLOT(slotOpen()));
			} else {
				popup->insertItem(i18n("Open"), this, SLOT(slotOpen()));
			}
		} else {
			if (mTarget[0] == '#' || mTarget[0] == '!' || mTarget[0] == '&')
			{
				popup->insertItem(i18n("Connect and Join"), this, SLOT(slotOpenConnect()));
			} else {
				popup->insertItem(i18n("Connect and Open"), this, SLOT(slotOpenConnect()));
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
	if (mContact->mWindow != 0)
	{
		mContact->mWindow->raise();
		if (mTabPage !=0)
		{
			mContact->mWindow->mTabWidget->showPage(mTabPage);
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
		if (mTarget[0] == '#' || mTarget[0] == '!' || mTarget[0] == '&')
		{
			if (chatView !=0)
			{
				chatView->chatView->append(partWarning);
				chatView->chatView->scrollToBottom();
			}
			waitingPart = true;
			mContact->engine->partChannel(QCString(mTarget.local8Bit()) ,QString("Using Kopete IRC Plugin"));
		}
	}
}

void IRCContact::slotPartedChannel(const QString &originating, const QString &channel, const QString &)
{
	if (mTarget.lower() == channel.lower() && originating.left(originating.find("!")).lower() == mContact->mNickname.lower())
	{
		unloading();
	}
}

IRCContact::~IRCContact()
{
	kopeteapp->contactList()->removeContact(this);
	if (mTarget[0] != '#' && mTarget[0] != '!' && mTarget[0] == '&')
	{
		mContact->activeQueries.remove(mTarget.lower());
	}
}

void IRCContact::unloading()
{
	if (mTabPage !=0)
	{
		delete mTabPage;
		mTabPage = 0L;
		chatView = 0L;
		queryView = 0L;
	}
}

void IRCContact::joinNow()
{
	kdDebug() << "IRC Plugin: IRCContact::joinNow() creating mTabPage!" << endl;
	mTabPage = new QVBox(mContact->mWindow->mTabWidget);
	if (mTarget[0] == '#' || mTarget[0] == '!' || mTarget[0] == '&')
	{
		chatView = new IRCChatView(mServer, mTarget, this, mTabPage);
		mContact->mWindow->mTabWidget->addTab(mTabPage, SmallIconSet("irc_privmsg.xpm"), mTarget);
	} else {
		queryView = new IRCQueryView(mServer, mTarget, mContact, mTabPage, this);
		mContact->mWindow->mTabWidget->addTab(mTabPage, SmallIconSet("irc_querymsg.xpm"), mTarget);
	}

	mContact->mWindow->show();

	KGlobal::config()->setGroup("IRC");
	bool minimize = KGlobal::config()->readBoolEntry("MinimizeNewQueries", false);
	if (mTarget[0] == '#' || mTarget[0] == '!' || mTarget[0] == '&')
	{
		mContact->mWindow->mTabWidget->showPage(mTabPage);
	} else {
		if (!minimize)
		{
			mContact->mWindow->mTabWidget->showPage(mTabPage);
		}
	}
}

void IRCContact::slotMoveThisUser() {
	QString mGroup = actionContactMove->currentText();

	kopeteapp->contactList()->moveContact(this, mGroup);

	mContact->mProtocol->mConfig->setGroup(mTarget.lower());
	mContact->mProtocol->mConfig->writeEntry("Group", mGroup);
	mContact->mProtocol->mConfig->sync();
}

void IRCContact::initActions()
{
	actionAddGroup = KopeteStdAction::addGroup( kopeteapp->contactList(), SLOT(addGroup()), this, "actionAddGroup" );
	actionContactMove = KopeteStdAction::moveContact( this, SLOT(slotMoveThisUser()), this, "actionMove" );
	actionRemove = KopeteStdAction::deleteContact( this, SLOT(removeThisUser()), this, "actionDelete" );
}
#include "irccontact.moc"
