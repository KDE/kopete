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

  the name convention is working|broken-plaintext|html-number.input|output
*/

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
	
	testLinkParser();
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

void KopeteMessage_Test::testLinkParser()
{
	QString basePath = QString::fromLatin1( SRCDIR ) + QString::fromLatin1("/link-parser-testcases");
	QDir testCasesDir(basePath);
	
	QStringList inputFileNames = testCasesDir.entryList("*.input");
	for ( QStringList::ConstIterator it = inputFileNames.begin(); it != inputFileNames.end(); ++it)
	{
		QString fileName = *it;
		QString outputFileName = fileName;
		outputFileName.replace("input","output");
		// open the input file
		QFile inputFile(basePath + QString::fromLatin1("/") + fileName);
		QFile expectedFile(basePath + QString::fromLatin1("/") + outputFileName);
		// check if the expected output file exists
		// if it doesn't, skip the testcase
		if ( ! expectedFile.exists() )
		{
			SKIP("Warning! expected output for testcase "+ *it + " not found. Skiping testcase");
			continue;
		}
		if ( inputFile.open( IO_ReadOnly ) && expectedFile.open( IO_ReadOnly ))
		{
			QTextStream inputStream(&inputFile);
			QTextStream expectedStream(&expectedFile);
			QString inputData;
			QString expectedData;
			inputData = inputStream.read();
			expectedData = expectedStream.read();

			inputFile.close();
			expectedFile.close();

			// use a concrete url
			inputData.replace( "$URL","http://www.kde.org" );
			expectedData.replace( "$URL","http://www.kde.org" );

			// set message format for parsing according to textcase filename convention
			Kopete::Message::MessageFormat format;
			if ( fileName.section("-", 1, 1) == QString::fromLatin1("plaintext") )
				format = Kopete::Message::PlainText;
			else
				format = Kopete::Message::RichText;		
	
			QString result = Kopete::Message::parseLinks( inputData, format );

			// HACK to know the test case we applied, concatenate testcase name to both
			// input and expected string. WIll remove when I can add some sort of metadata
			// to a CHECK so debug its origin testcase
			result = fileName + QString::fromLatin1(": ") + result;
			expectedData = fileName + QString::fromLatin1(": ") + expectedData;
			// if the test case begins with broken, we expect it to fail, then use XFAIL
			// otherwise use CHECK
			if ( fileName.section("-", 0, 0) == QString::fromLatin1("broken") )
			{
				kdDebug() << "checking known-broken testcase: " << fileName << endl;
				XFAIL(result, expectedData);
			}
			else
			{
				kdDebug() << "checking known-working testcase: " << fileName << endl;
				CHECK(result, expectedData);
			}
		}
		else
		{
			SKIP("Warning! can't open testcase files for "+ *it + ". Skiping testcase");
			continue;
		}
	}
	
}
