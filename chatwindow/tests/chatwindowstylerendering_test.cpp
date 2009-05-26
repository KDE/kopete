/*
    Adium(and Kopete) ChatWindowStyle format rendering test suite

    Copyright (c) 2005      by Michaël Larouche       <larouche@kde.org>

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
#include <qtextstream.h>

// KDE includes
#include <kunittest/module.h>
#include <kcomponentdata.h>
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
FakeProtocol( const KComponentData &instance, QObject *parent, const char *name ) : Kopete::Protocol(instance, parent, name)
{
	
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

void setOnlineStatus( const Kopete::OnlineStatus& /*status*/, const Kopete::StatusMessage &/*reason*/, const OnlineStatusOptions&/*options*/)
{
	// do nothing
}
};

class ChatWindowStyleRendering_Test::Private
{
public:
	Private()
	{
		protocol = new FakeProtocol( KComponentData(QCString("test-kopete-message")), 0L, "test-kopete-message");
		account = new FakeAccount(protocol, QString("testaccount"), 0);

		// Create fake meta/contacts
		myselfMetaContact = new Kopete::MetaContact();
		myself = new FakeContact(account, "bob@localhost", myselfMetaContact);
		myself->setNickName("Bob");
		otherMetaContact = new Kopete::MetaContact();
		other = new FakeContact(account, "audrey@localhost", otherMetaContact);
		other->setNickName("Audrey");
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
	setenv( "KDEHOME", QFile::encodeName( QDir::homePath() + "/.kopete-unittest" ), true );

	//KApplication::disableAutoDcopRegistration();
	//KCmdLineArgs::init(argc,argv,"testkopetemessage", 0, KLocalizedString(), 0, KLocalizedString());
	KApplication app;

	chatPart = new ChatMessagePart(d->fakeChatSession, 0, 0);

	testHeaderRendering();
	testMessageRendering();
	testStatusRendering();
	//testFullRendering();
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
	.arg(QDateTime::currentDateTime().toString("hh:mm"));

	QString headerHtml = d->testStyle->getHeaderHtml();
	QString resultHtml;

	resultHtml = chatPart->formatStyleKeywords(headerHtml);

	kDebug(14000) << "Result HTML: " << resultHtml;

	CHECK(resultHtml, expectedHtml);
}

void ChatWindowStyleRendering_Test::testMessageRendering()
{
	QString expectedIncomingHtml = QString(
"Incoming:\n"
"<div>Incoming/buddy_icon.png</div>\n"
"<div>audrey@localhost</div>\n"
"<div>Audrey</div>\n"
"<div>Kopete</div>\n"
"<div>Test</div>\n"
"<div>%1</div>\n"
"<div>%2</div>\n"
"<div id=\"insert\">"
	).arg(KGlobal::locale()->formatDateTime( QDateTime::currentDateTime()))
	.arg(QDateTime::currentDateTime().toString("hh:mm"));

	QString expectedOutgoingHtml = QString(
"Outgoing:\n"
"<div>Outgoing/buddy_icon.png</div>\n"
"<div>bob@localhost</div>\n"
"<div>Bob</div>\n"
"<div>Kopete</div>\n"
"<div>Hello there</div>\n"
"<div>%1</div>\n"
"<div>%2</div>\n"
"<div id=\"insert\">"
	).arg(KGlobal::locale()->formatDateTime( QDateTime::currentDateTime()))
	.arg(QDateTime::currentDateTime().toString("hh:mm"));


	QString tempHtml;
	QString resultHtml;
	
	Kopete::Message msgIn(d->other, d->myself, QString::fromUtf8("Test"), Kopete::Message::Inbound );
	Kopete::Message msgOut(d->myself, d->other, QString::fromUtf8("Hello there"), Kopete::Message::Outbound);

	tempHtml = d->testStyle->getIncomingHtml();
	resultHtml = chatPart->formatStyleKeywords(tempHtml, msgIn);

	kDebug(14000) << "Message incoming HTML: " << resultHtml;

	CHECK(resultHtml, expectedIncomingHtml);

	tempHtml = d->testStyle->getOutgoingHtml();
	resultHtml = chatPart->formatStyleKeywords(tempHtml, msgOut);

	kDebug(14000) << "Message outgoing HTML: " << resultHtml;

	CHECK(resultHtml, expectedOutgoingHtml);
}

void ChatWindowStyleRendering_Test::testStatusRendering()
{
	QString expectedStatusHtml = QString(
"<div>A contact went offline.</div>\n"
"<div>%1</div>\n"
"<div>%2</div>"
	).arg(KGlobal::locale()->formatDateTime( QDateTime::currentDateTime()))
	.arg(QDateTime::currentDateTime().toString("hh:mm"));

	QString statusHtml = d->testStyle->getStatusHtml();
	QString resultHtml;
	
	Kopete::Message msgStatus(0,0, QString::fromUtf8("A contact went offline."), Kopete::Message::Internal);
	resultHtml = chatPart->formatStyleKeywords(statusHtml, msgStatus);

	CHECK(resultHtml, expectedStatusHtml);
}

void ChatWindowStyleRendering_Test::testFullRendering()
{
	QString expectedFullHtml;
	QString resultHtml;

	Kopete::Message msgIn1(d->myself, d->other, QString("Hello there !"), Kopete::Message::Inbound);
	Kopete::Message msgIn2(d->myself, d->other, QString("How are you doing ?"), Kopete::Message::Inbound);
	Kopete::Message msgOut1(d->other, d->myself, QString("Fine and you ?"), Kopete::Message::Outbound);
	Kopete::Message msgStatus1(d->myself,d->other, QString("You are now marked as away."), Kopete::Message::Internal);
	Kopete::Message msgStatus2(d->myself,d->other, QString("You are now marked as online."), Kopete::Message::Internal);
	Kopete::Message msgIn3(d->myself, d->other, QString("Well, doing some tests."), Kopete::Message::Internal);
	Kopete::Message msgOut2(d->other, d->myself, QString("All your bases are belong to us."), Kopete::Message::Outbound);
	Kopete::Message msgOut3(d->other, d->myself, QString("You are on the way to destruction"), Kopete::Message::Outbound);

	// Change style on the fly in ChatMessagePart so this test would run
	chatPart->setStyle(d->testStyle);
	
	// Simulate a consersation
	chatPart->appendMessage(msgIn1);
	chatPart->appendMessage(msgIn2);
	chatPart->appendMessage(msgOut1);
	chatPart->appendMessage(msgStatus1);
	chatPart->appendMessage(msgStatus2);
	chatPart->appendMessage(msgIn3);
	chatPart->appendMessage(msgOut2);
	chatPart->appendMessage(msgOut3);

	resultHtml = chatPart->htmlDocument().toHTML();

	// Read the expected(sample) HTML from file.
	QFile sampleHtml(QString(SRCDIR)+"sample.html");
	if(sampleHtml.open(QIODevice::ReadOnly))
	{
		QTextStream stream(&sampleHtml);
		stream.setEncoding(QTextStream::UnicodeUTF8);
		expectedFullHtml = stream.read();
		sampleHtml.close();
	}

	CHECK(resultHtml, expectedFullHtml);
}
