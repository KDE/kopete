/*
    Adium(and Kopete) ChatWindowStyle format rendering test suite

    Copyright (c) 2005      by Michaël Larouche       <michael.larouche@kdemail.net>

    Kopete    (c) 2002-2005 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/


#include "chatwindowstylerendering_test.h"

#include <stdlib.h>

// Qt includes
#include <qdir.h>
#include <qfile.h>
#include <qdatetime.h>

// KDE includes
#include <kapplication.h>
#include <kunittest/module.h>
#include <kinstance.h>
#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>

#include <kopetechatwindowstyle.h>

// Libkopete includes
#include <kopeteprotocol.h>
#include <kopetemetacontact.h>
#include <kopeteaccount.h>
#include <kopetecontact.h>
#include <kopetechatsession.h>
#include <kopetechatsessionmanager.h>

using namespace Kopete;

KUNITTEST_MODULE( kunittest_chatwindowstylerendering_test, "KopeteChatWindowTestSuite");
KUNITTEST_MODULE_REGISTER_TESTER( ChatWindowStyleRendering_Test );

// Reimplement Kopete::Contact and its abstract method
class FakeContact : public Kopete::Contact
{
public:
	FakeContact (Kopete::Account *account, const QString &id, Kopete::MetaContact *mc ) : Kopete::Contact( account, id, mc ) {}
	virtual Kopete::ChatSession *manager(Kopete::Contact::CanCreateFlags /*c*/) { return 0L; }
	virtual void slotUserInfo() {};
};

class FakeProtocol : public Kopete::Protocol
{
public:
FakeProtocol( KInstance *instance, QObject *parent, const char *name ) : Kopete::Protocol(instance, parent, name)
{
	
}

QString displayName() const
{
	return QString("FakeProtocol");
}
QString pluginIcon () const
{
	return QString("kopete");
}

Account* createNewAccount( const QString &/*accountId*/ )
{
	return 0L;
}

AddContactPage* createAddContactWidget( QWidget */*parent*/, Kopete::Account */*account*/)
{
	return 0L;
}

KopeteEditAccountWidget* createEditAccountWidget( Kopete::Account */*account*/, QWidget */*parent */)
{
	return 0L;
}

};

class FakeAccount : public Kopete::Account
{
public:
FakeAccount(Kopete::Protocol *parent, const QString &accountID, const char *name) : Kopete::Account(parent, accountID, name)
{

}

~FakeAccount()
{

}

bool createContact( const QString &/*contactId*/, Kopete::MetaContact */*parentContact*/ )
{
	return true;
}

void connect( const Kopete::OnlineStatus& /*initialStatus*/)
{
	// do nothing
}

void disconnect()
{
	// do nothing
}

void setOnlineStatus( const Kopete::OnlineStatus& /*status*/ , const QString &/*reason*/)
{
	// do nothing
}
};

class ChatWindowStyleRendering_Test::Private
{
public:
	Private()
	{
		protocol = new FakeProtocol( new KInstance(QCString("test-kopete-message")), 0L, "test-kopete-message");
		account = new FakeAccount(protocol, QString("testaccount"), 0);

		// Create fake meta/contacts
		myselfMetaContact = new Kopete::MetaContact();
		myself = new FakeContact(account, "bob@localhost", myselfMetaContact);
		otherMetaContact = new Kopete::MetaContact();
		other = new FakeContact(account, "audrey@localhost", otherMetaContact);
		myselfMetaContact->setDisplayName("Bob");
		myselfMetaContact->setDisplayNameSource(Kopete::MetaContact::SourceCustom);
		otherMetaContact->setDisplayName("Audrey");
		otherMetaContact->setDisplayNameSource(Kopete::MetaContact::SourceCustom);

		Kopete::ContactPtrList contactList;
		contactList.append(other);
		// Create fakeChatSession
		fakeChatSession = Kopete::ChatSessionManager::self()->create(myself, contactList, 0);
		fakeChatSession->setDisplayName("Test Session");

		// Create testStyle
		testStyle = new ChatWindowStyle(QString(SRCDIR)+QString("/TestStyle"));
	}
	~Private()
	{
		delete myselfMetaContact;
		delete otherMetaContact;
		delete myself;
		delete other;
		delete fakeChatSession;
	}
	FakeProtocol *protocol;
	FakeAccount *account;
	Kopete::MetaContact *myselfMetaContact;
	Kopete::MetaContact *otherMetaContact;
	FakeContact *myself;
	FakeContact *other;
	Kopete::ChatSession *fakeChatSession;
	ChatWindowStyle *testStyle;

	QString resultHTML;
};



ChatWindowStyleRendering_Test::ChatWindowStyleRendering_Test()
{
	d = new Private;
}

ChatWindowStyleRendering_Test::~ChatWindowStyleRendering_Test()
{
	delete d;
}

void ChatWindowStyleRendering_Test::allTests()
{
	// change user data dir to avoid messing with user's .kde dir
	setenv( "KDEHOME", QFile::encodeName( QDir::homeDirPath() + "/.kopete-unittest" ), true );

	KApplication::disableAutoDcopRegistration();
	//KCmdLineArgs::init(argc,argv,"testkopetemessage", 0, 0, 0, 0);
	KApplication app;

	chatPart = new ChatMessagePart(d->fakeChatSession, 0, 0);

	testHeaderRendering();
}

void ChatWindowStyleRendering_Test::testHeaderRendering()
{
	QString expectedHtml = QString(
"<div>Test Session</div>\n"
"<div>Bob</div>\n"
"<div>Audrey</div>\n"
"<div>Incoming/buddy_icon.png</div>\n"
"<div>Outgoing/buddy_icon.png</div>\n"
"<div>%1</div>\n"
"<div>%2</div>"
	).arg(KGlobal::locale()->formatDateTime( QDateTime::currentDateTime()))
	.arg(QDateTime::currentDateTime().toString("h:m"));

	QString headerHtml = d->testStyle->getHeaderHtml();
	QString resultHtml;

	resultHtml = chatPart->formatStyleKeywords(headerHtml);

	kdDebug(14000) << "Result HTML: " << resultHtml << endl;

	CHECK(resultHtml, expectedHtml);
}
