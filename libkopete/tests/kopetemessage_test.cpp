/*
    Tests for Kopete::Message

    Copyright (c) 2005 by Duncan Mac-Vicar Prett  <duncan@kde.org>
		Copyright (c) 2004 by Richard Smith <kde@metafoo.co.uk>

    Kopete (c) 2002-2005 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include <stdlib.h>

#include <qdir.h>
#include <qfile.h>
#include <kapplication.h>
#include <kstandarddirs.h>
#include <kinstance.h>
#include <kprocess.h>
#include <kunittest/module.h>
#include <kdebug.h>

#include "kopetemessage_test.h"
#include "kopeteaccount_mock.h"
#include "kopeteprotocol_mock.h"
#include "kopetecontact_mock.h"
#include "kopetemetacontact_mock.h"
#include "kopeteaccount_mock.h"

using namespace KUnitTest;

KUNITTEST_MODULE( kunittest_kopetemessage_test, "KopeteSuite");
KUNITTEST_MODULE_REGISTER_TESTER( KopeteMessage_Test );

/*
  There are four sets of tests: for each of plain text and html, we have those
  known to work in the current codebase, and those known to fail right now.

  Each entry in a set consists of two strings. The first string is the input,
  the second string is the EXPECTED output. If the two differ the test is
  considered failed and the actual result is output for comparison.

  A set should end with an entry containing at least one null pointer.
*/

typedef const char * LinkTestSet[][ 2 ];

static LinkTestSet knownGoodPlain =
{
	{ "$URL", "<a href=\"$URL\" title=\"$URL\">$URL</a>" },
	{ "$URL/", "<a href=\"$URL/\" title=\"$URL/\">$URL/</a>" },
	{ "www.kde.org/", "<a href=\"$URL/\" title=\"$URL/\">www.kde.org/</a>" },
	{ NULL, NULL }
};

static LinkTestSet knownBrokenPlain =
{
	{ NULL, NULL }
};

static LinkTestSet knownGoodHTML =
{
	{ "$URL", "<a href=\"$URL\" title=\"$URL\">$URL</a>" },
	{ "<a href=\"$URL\">KDE</a>", "<a href=\"$URL\">KDE</a>" },
	{ NULL, NULL }
};

static LinkTestSet knownBrokenHTML =
{
	{ "<a href=\"$URL\" title=\"$URL\">$URL</a>", "<a href=\"$URL\" title=\"$URL\">$URL</a>" },
	{ NULL, NULL }
};


KopeteMessage_Test::KopeteMessage_Test()
{
	setup();
}

void KopeteMessage_Test::allTests()
{
	// change user data dir to avoid messing with user's .kde dir
	setenv( "KDEHOME", QFile::encodeName( QDir::homeDirPath() + "/.kopete-unittest" ), true );

	KApplication::disableAutoDcopRegistration();
	//KCmdLineArgs::init(argc,argv,"testkopetemessage", 0, 0, 0, 0);
	KApplication app;
	
	// create fake objects needed to build a reasonable testeable message
	m_protocol = new Kopete::Test::Mock::Protocol( new KInstance(QCString("test-kopete-message")), 0L, "test-kopete-message");
	m_account = new Kopete::Test::Mock::Account(m_protocol, "testaccount");
	m_metaContactMyself = new Kopete::Test::Mock::MetaContact();
	m_metaContactOther = new Kopete::Test::Mock::MetaContact();
	m_contactFrom = new Kopete::Test::Mock::Contact(m_account, QString::fromLatin1("test-myself"), m_metaContactMyself, QString::null);
	m_contactTo = new Kopete::Test::Mock::Contact(m_account, QString::fromLatin1("test-dest"), m_metaContactOther, QString::null);
	m_message = new Kopete::Message( m_contactFrom, m_contactTo, QString::null, Kopete::Message::Outbound, Kopete::Message::PlainText);
	//testKnownGoodHTML();
	//testKnownBrokenHTML();
	//testKnownGoodPlain();
	//testKnownBrokenPlain();
	testValidXML();
}

void KopeteMessage_Test::testFormats()
{
	
}

void KopeteMessage_Test::testValidXML()
{
	if ( KStandardDirs::findExe( QString::fromLatin1("xmllint") ).isEmpty() )
	{
		SKIP("Sorry, this test requires xmllint installed.");
	}

	Kopete::Test::Mock::Contact* contactFrom = new Kopete::Test::Mock::Contact( 0L /*account*/, QString::fromLatin1("test-friend"), 0L /* metaContact */);
	Kopete::Test::Mock::Contact* contactTo = new Kopete::Test::Mock::Contact( 0L /*account*/, QString::fromLatin1("test-myself"), 0L /* metaContact */);
	
	Kopete::Message message( contactFrom, contactTo, QString::fromLatin1("Hello my friend, I am Testing you"), Kopete::Message::Inbound, Kopete::Message::PlainText);
	
	kdDebug(14010) << k_funcinfo << endl;
	QString xml = message.asXML().toString();
	QFile xmlFile("message.xml");
	if ( xmlFile.open( IO_WriteOnly ) )
	{
		kdDebug(14010) << k_funcinfo << "Writing xml" << endl;
		QTextStream outXML(&xmlFile);
		outXML << QString::fromLatin1("<?xml version=\"1.0\"?>\n");
		outXML << xml;	
		xmlFile.close();
	}
	else
	{
		kdDebug(14010) << k_funcinfo << "Cannot open file" << endl;
	}

	QString schemaPath = QString::fromLatin1( SRCDIR ) + QString::fromLatin1("/kopetemessage.xsd");
	kdDebug() << k_funcinfo << schemaPath << endl;
	KProcess p;
	p << "xmllint" << "--noout" << "--schema" << schemaPath << QString::fromLatin1("message.xml"); 
	p.start(KProcess::Block);
	if (p.normalExit())
	{
		//kdDebug() << k_funcinfo << p.exitStatus();
		// Exit code 0 NO ERROR on validating.
		CHECK( p.exitStatus(), 0 );
	}
	delete contactTo;
	delete contactFrom;
}

void KopeteMessage_Test::setup()
{
	
}

void KopeteMessage_Test::testKnownGoodHTML()
{
	uint i = 0;
	while ( knownGoodHTML[ i ][ 0 ] && knownGoodHTML[ i ][ 1 ] )
	{
		QString input = knownGoodHTML[ i ][ 0 ], expected = knownGoodHTML[ i ][ 1 ];
		input.replace( "$URL","http://www.kde.org" );
		expected.replace( "$URL","http://www.kde.org" );
		QString result = Kopete::Message::parseLinks( input, Kopete::Message::RichText );
	
		CHECK(result, expected);
		i++;
	}
}

void KopeteMessage_Test::testKnownBrokenHTML()
{
	uint i = 0;
	while ( knownBrokenHTML[ i ][ 0 ] && knownBrokenHTML[ i ][ 1 ] )
	{
		QString input = knownBrokenHTML[ i ][ 0 ], expected = knownBrokenHTML[ i ][ 1 ];
		input.replace( "$URL","http://www.kde.org" );
		expected.replace( "$URL","http://www.kde.org" );
		QString result = Kopete::Message::parseLinks( input, Kopete::Message::RichText );
	
		XFAIL(result, expected);
		i++;
	}
}
void KopeteMessage_Test::testKnownGoodPlain()
{
	uint i = 0;
	while ( knownGoodPlain[ i ][ 0 ] && knownGoodPlain[ i ][ 1 ] )
	{
		QString input = knownGoodPlain[ i ][ 0 ], expected = knownGoodPlain[ i ][ 1 ];
		input.replace( "$URL","http://www.kde.org" );
		expected.replace( "$URL","http://www.kde.org" );
		QString result = Kopete::Message::parseLinks( input, Kopete::Message::PlainText );
	
		CHECK(result, expected);
		i++;
	}
}

void KopeteMessage_Test::testKnownBrokenPlain()
{
	uint i = 0;
	while ( knownBrokenPlain[ i ][ 0 ] && knownBrokenPlain[ i ][ 1 ] )
	{
		QString input = knownBrokenPlain[ i ][ 0 ], expected = knownBrokenPlain[ i ][ 1 ];
		input.replace( "$URL","http://www.kde.org" );
		expected.replace( "$URL","http://www.kde.org" );
		QString result = Kopete::Message::parseLinks( input, Kopete::Message::PlainText );
	
		XFAIL(result, expected);
		i++;
	}
}
