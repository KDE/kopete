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

IRCContact::IRCContact(const QString &server, const QString &target, unsigned int port, bool joinOnConnect, IRCServerContact *contact)
	: KopeteContact(contact->mProtocol)
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

	connect(mContact->engine, SIGNAL(connectionClosed()), this, SLOT(slotServerQuit()));

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

	connect(mContact->engine, SIGNAL(connectionClosed()), this, SLOT(slotServerQuit()));

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

	kopeteapp->contactList()->addContact(this, groupName);
	setName(QString("%1@%2").arg(target).arg(mServer));

	connect(mContact->engine, SIGNAL(connectionClosed()), this, SLOT(slotServerQuit()));

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

void IRCContact::slotServerQuit()
{
	if (mTabPage != 0)
	{
		delete mTabPage;
		mTabPage = 0L;
		queryView = 0L;
		chatView = 0L;
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
			if (mContact->mWindow != 0 && mContact->mWindow->mTabWidget != 0)
			{
				for (int i = 0; i != mContact->mWindow->mTabWidget->count(); i++)
				{
					if (mContact->mWindow->mTabWidget->label(i).lower() == mTarget.lower())
					{
						QWidget *page = mContact->mWindow->mTabWidget->page(i);
						mContact->mWindow->show();
						if (page != 0)
						{
							mContact->mWindow->mTabWidget->showPage(page);
							break;
						}
					}
				}
			}
			delete this;
			return;
		}
		if (mContact->activeQueries.find(mTarget.lower()) == mContact->activeQueries.end())
		{
			mContact->activeQueries.append(mTarget.lower());
		}
	}
	connect(mContact, SIGNAL(quittingServer()), this, SLOT(slotServerIsQuitting()));
	connect(mContact, SIGNAL(serverQuit()), this, SLOT(slotServerHasQuit()));
	connect(mContact->engine, SIGNAL(incomingPartedChannel(const QString &, const QString &, const QString &)), this, SLOT(slotPartedChannel(const QString &, const QString &, const QString &)));

}

void IRCContact::slotServerHasQuit()
{
	unloading();
}

void IRCContact::slotServerIsQuitting()
{
	if (requestedQuit == false)
	{
		QColor color(175, 8, 8);
		QString partWarning = "<font color=";
		partWarning.append(color.name());
		partWarning.append(i18n(">Attempting to quit server. If this takes an unusual amount of time, please click the red stop button on the toolbar.</font><br>"));
		if (mTarget[0] == '#' || mTarget[0] == '!' || mTarget[0] == '&')
		{
			if (chatView !=0)
			{
				chatView->chatView->append(partWarning);
				chatView->chatView->scrollToBottom();
			}
		}
	}
}

void IRCContact::slotRemoveThis()
{
	if (mTarget[0] == '#' || mTarget[0] == '!' || mTarget[0] == '&')
	{
		return;
	}
	for (int i = 0; i != mContact->mWindow->mTabWidget->count(); i++)
	{
		if (mContact->mWindow->mTabWidget->count() <= 0)
		{
			break;
		}
		if (mContact->mWindow->mTabWidget->label(i).lower() == mTarget.lower())
		{
			QWidget *page = mContact->mWindow->mTabWidget->page(i);
			if (page != 0)
			{
				mContact->mWindow->mTabWidget->removePage(page);
				if (mTabPage !=0)
				{
					delete mTabPage;
					mTabPage = 0L;
					queryView = 0L;
					chatView = 0L;
				}
				delete this;
				return;
			}
		}
		if (i > 1000)
		{
			// just make sure that it doesn't cause an infinite loop, and since people aren't going to be in 1000 channel or queries on server, it's okay
			break;
		}
	}
	delete this;
}

void IRCContact::slotClose()
{
	if (mTabPage != 0)
	{
		delete mTabPage;
		mTabPage = 0L;
		queryView = 0L;
		chatView = 0L;
	}
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

void IRCContact::showContextMenu(QPoint point)
{
	popup = new KPopupMenu();
	popup->insertTitle(mTarget);
	if (mTarget[0] == '#' || mTarget[0] == '!' || mTarget[0] == '&')
	{
		popup->insertItem("Part", this, SLOT(slotPart()));
// TODO:	popup->insertItem("Hop (Part and Re-join)", this, SLOT(slotHop()));
// TODO:	popup->insertItem("Remove", this, SLOT(slotRemoveThis()));
	} else {
		if (mTabPage != 0)
		{
			popup->insertItem("Close", this, SLOT(slotClose()));
		} else {
			if (mContact->engine->isLoggedIn())
			{
				popup->insertItem("Open", this, SLOT(slotOpen()));
			} else {
				popup->insertItem("Open and Connect", this, SLOT(slotOpenConnect()));
			}
		}
		popup->insertItem("Remove", this, SLOT(slotRemoveThis()));
	}
	popup->popup(point);
}

void IRCContact::slotQuitServer()
{
	requestedQuit = true;
	mContact->slotQuitServer();
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
	mContact->activeContacts.remove(this);
	kopeteapp->contactList()->removeContact(this);
	mContact->activeQueries.remove(mTarget);

}

void IRCContact::unloading()
{
	if (mTabPage !=0)
	{
		delete mTabPage;
		mTabPage = 0L;
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
		QObject::connect(mContact->engine, SIGNAL(userJoinedChannel(const QString &, const QString &)), chatView, SLOT(userJoinedChannel(const QString &, const QString &)));
		QObject::connect(mContact->engine, SIGNAL(incomingMessage(const QString &, const QString &, const QString &)), chatView, SLOT(incomingMessage(const QString &, const QString &, const QString &)));
		QObject::connect(mContact->engine, SIGNAL(incomingPartedChannel(const QString &, const QString &, const QString &)), chatView, SLOT(userPartedChannel(const QString &, const QString &, const QString &)));
		QObject::connect(mContact->engine, SIGNAL(incomingNamesList(const QString &, const QString &, const int)), chatView, SLOT(incomingNamesList(const QString &, const QString &, const int)));
		QObject::connect(mContact->engine, SIGNAL(incomingAction(const QString &, const QString &, const QString &)), chatView, SLOT(incomingAction(const QString &, const QString &, const QString &)));
		QObject::connect(mContact->engine, SIGNAL(incomingQuitIRC(const QString &,  const QString &)), chatView, SLOT(userQuitIRC(const QString &, const QString &)));
		QObject::connect(mContact->engine, SIGNAL(incomingNickChange(const QString &,  const QString &)), chatView, SLOT(nickNameChanged(const QString &, const QString &)));
		QObject::connect(mContact->engine, SIGNAL(incomingTopicChange(const QString &, const QString &, const QString &)), chatView, SLOT(incomingNewTopic(const QString &, const QString &, const QString &)));
		QObject::connect(mContact->engine, SIGNAL(incomingExistingTopic(const QString &,  const QString &)), chatView, SLOT(receivedExistingTopic(const QString &, const QString &)));
		QObject::connect(mContact->engine, SIGNAL(incomingNoNickChan(const QString &)), chatView, SLOT(incomingNoNickChan(const QString &)));
	} else {
		queryView = new IRCQueryView(mServer, mTarget, mContact, mTabPage, this);
		mContact->mWindow->mTabWidget->addTab(mTabPage, SmallIconSet("irc_querymsg.xpm"), mTarget);
		QObject::connect(mContact->engine, SIGNAL(incomingMessage(const QString &, const QString &, const QString &)), queryView, SLOT(incomingMessage(const QString &, const QString &, const QString &)));
		QObject::connect(mContact->engine, SIGNAL(incomingAction(const QString &, const QString &, const QString &)), queryView, SLOT(incomingAction(const QString &, const QString &, const QString &)));
		QObject::connect(mContact->engine, SIGNAL(incomingNoNickChan(const QString &)), queryView, SLOT(incomingNoNickChan(const QString &)));
	}

	mContact->mWindow->show();
	mContact->mWindow->mTabWidget->showPage(mTabPage);
}
#include "irccontact.moc"
