/*
    Tests for Kopete::Message

    Copyright (c) 2005   by Tommi Rantala  <tommi.rantala@cs.helsinki.fi>
    Copyright (c) 2005   by Duncan Mac-Vicar Prett  <duncan@kde.org>
    Copyright (c) 2004   by Richard Smith <kde@metafoo.co.uk>

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
	// change user data dir to avoid messing with user's .kde dir
	setenv( "KDEHOME", QFile::encodeName( QDir::homeDirPath() + "/.kopete-unittest" ), true );

	// create fake objects needed to build a reasonable testeable message
	m_protocol = new Kopete::Test::Mock::Protocol( new KInstance(QCString("test-kopete-message")), 0L, "test-kopete-message");
	m_account = new Kopete::Test::Mock::Account(m_protocol, "testaccount");
	m_metaContactMyself = new Kopete::Test::Mock::MetaContact();
	m_metaContactOther = new Kopete::Test::Mock::MetaContact();
	m_contactFrom = new Kopete::Test::Mock::Contact(m_account, QString::fromLatin1("test-myself"), m_metaContactMyself, QString::null);
	m_contactTo = new Kopete::Test::Mock::Contact(m_account, QString::fromLatin1("test-dest"), m_metaContactOther, QString::null);
	m_message = new Kopete::Message( m_contactFrom, m_contactTo, QString::null, Kopete::Message::Outbound, Kopete::Message::PlainText);
}

void KopeteMessage_Test::allTests()
{
	KApplication::disableAutoDcopRegistration();
	//KCmdLineArgs::init(argc,argv,"testkopetemessage", 0, 0, 0, 0);

	// At least Kopete::Message::asXML() seems to require that a QApplication
	// is created. Running the console version doesn't create it, but the GUI
	// version does.

	if (!kapp)
		new KApplication();

	testPrimitives();
	testLinkParser();
}

void KopeteMessage_Test::testPrimitives()
{
	/**********************************************
	 * from(), to()
	 *********************************************/

	{
		Kopete::Message msg( m_contactFrom, m_contactTo, "foobar", Kopete::Message::Inbound, Kopete::Message::PlainText);
		Q_ASSERT(msg.from());
		Q_ASSERT(!msg.to().isEmpty());
	}

	/**********************************************
	 * Direction
	 *********************************************/

	{
		Kopete::Message msg( m_contactFrom, m_contactTo, "foobar", Kopete::Message::Inbound, Kopete::Message::PlainText);
		CHECK(Kopete::Message::Inbound, msg.direction());
	}
	{
		Kopete::Message msg( m_contactFrom, m_contactTo, "foobar", Kopete::Message::Outbound, Kopete::Message::RichText);
		CHECK(Kopete::Message::Outbound, msg.direction());
	}
	{
		Kopete::Message msg( m_contactFrom, m_contactTo, "foobar", Kopete::Message::Internal, Kopete::Message::RichText);
		CHECK(Kopete::Message::Internal, msg.direction());
	}

	/**********************************************
	 * Message Format
	 *********************************************/

	{
		Kopete::Message msg( m_contactFrom, m_contactTo, "foobar", Kopete::Message::Inbound, Kopete::Message::PlainText);
		CHECK(Kopete::Message::PlainText, msg.format());
	}
	{
		Kopete::Message msg( m_contactFrom, m_contactTo, "foobar", Kopete::Message::Inbound, Kopete::Message::RichText);
		CHECK(Kopete::Message::RichText, msg.format());
	}
	{
		QString m = "foobar";
		Kopete::Message msg( m_contactFrom, m_contactTo, m, Kopete::Message::Inbound, Kopete::Message::RichText);

		msg.setBody(m, Kopete::Message::PlainText);
		CHECK(Kopete::Message::PlainText, msg.format());

		msg.setBody(m, Kopete::Message::RichText);
		CHECK(Kopete::Message::RichText, msg.format());

		msg.setBody(m, Kopete::Message::ParsedHTML);
		CHECK(Kopete::Message::ParsedHTML, msg.format());

		msg.setBody(m, Kopete::Message::Crypted);
		CHECK(Kopete::Message::Crypted, msg.format());
	}


	/**********************************************
	 * setBody()
	 *********************************************/

	{
		QString m = "foobar";
		Kopete::Message msg( m_contactFrom, m_contactTo, m, Kopete::Message::Inbound, Kopete::Message::RichText);

		msg.setBody("NEW", Kopete::Message::PlainText);
		CHECK(QString("NEW"), msg.plainBody());

		msg.setBody("NEW_NEW", Kopete::Message::RichText);
		CHECK(QString("NEW_NEW"), msg.plainBody());
	}
	{
		QString m = "foobar";
		Kopete::Message msg( m_contactFrom, m_contactTo, m, Kopete::Message::Inbound, Kopete::Message::PlainText);

		msg.setBody("NEW", Kopete::Message::PlainText);
		CHECK(QString("NEW"), msg.plainBody());

		msg.setBody("NEW_NEW", Kopete::Message::RichText);
		CHECK(QString("NEW_NEW"), msg.plainBody());
	}
	{
		QString m = "<html><head></head><body foo=\"bar\">   <b>HELLO WORLD</b>   </body></html>";
		Kopete::Message msg( m_contactFrom, m_contactTo, m, Kopete::Message::Inbound, Kopete::Message::PlainText);
		CHECK(m, msg.plainBody());

		msg.setBody("<simple> SIMPLE", Kopete::Message::PlainText);
		CHECK(msg.plainBody(),   QString("<simple> SIMPLE") );
		CHECK(msg.escapedBody(), QString("&lt;simple&gt; SIMPLE") );

		msg.setBody("<simple>SIMPLE</simple>", Kopete::Message::RichText);
		CHECK(msg.plainBody(),   QString("SIMPLE") );
		CHECK(msg.escapedBody(), QString("<simple>SIMPLE</simple>") );

		CHECK(Kopete::Message::unescape( QString( "<simple>SIMPLE</simple>" ) ), QString("SIMPLE") );
		CHECK(Kopete::Message::unescape( QString( "Foo <img src=\"foo.png\" />" ) ), QString("Foo ") );
		CHECK(Kopete::Message::unescape( QString( "Foo <img src=\"foo.png\" title=\"Bar\" />" ) ), QString("Foo Bar") );

		msg.setBody(m, Kopete::Message::RichText);

		// FIXME: Should setBody() also strip extra white space?
		//CHECK(msg.plainBody(),   QString("HELLO WORLD"));
		//CHECK(msg.escapedBody(), QString("<b>HELLO WORLD</b>"));

		CHECK(msg.escapedBody(),                   QString(" &nbsp; <b>HELLO WORLD</b> &nbsp; "));
		CHECK(msg.plainBody(),                     QString("   HELLO WORLD   "));
		CHECK(msg.plainBody().stripWhiteSpace(),   QString("HELLO WORLD"));
		CHECK(msg.escapedBody().stripWhiteSpace(), QString("&nbsp; <b>HELLO WORLD</b> &nbsp;"));
	}
	{
		Kopete::Message msg( m_contactFrom, m_contactTo, "foo", Kopete::Message::Inbound, Kopete::Message::PlainText);

		msg.setBody("<p>foo", Kopete::Message::RichText);
		CHECK(msg.escapedBody(), QString("foo"));

		msg.setBody("<p>foo</p>", Kopete::Message::RichText);
		CHECK(msg.escapedBody(), QString("foo"));

		msg.setBody("\n<p>foo</p>\n<br/>", Kopete::Message::RichText);
		CHECK(msg.escapedBody(), QString("foo<br/>"));
	}

	/**********************************************
	 * Copy constructor
	 *********************************************/

	{
		Kopete::Message msg1(m_contactFrom, m_contactTo, "foo", Kopete::Message::Inbound, Kopete::Message::RichText);
		Kopete::Message msg2(msg1);

		CHECK(msg1.plainBody(), msg2.plainBody());
		CHECK(msg1.escapedBody(), msg2.escapedBody());

		msg1.setBody("NEW", Kopete::Message::PlainText);
		CHECK(msg1.plainBody(), QString("NEW"));
		CHECK(msg2.plainBody(), QString("foo"));
	}

	/**********************************************
	 * operator=
	 *********************************************/

	{
		Kopete::Message msg1(m_contactFrom, m_contactTo, "foo", Kopete::Message::Inbound, Kopete::Message::RichText);
		{
			Kopete::Message msg2;

			CHECK(msg2.plainBody(), QString::null);

			msg2 = msg1;

			CHECK(msg1.plainBody(), msg2.plainBody());
			CHECK(msg1.escapedBody(), msg2.escapedBody());

			msg1.setBody("NEW", Kopete::Message::PlainText);
			CHECK(msg1.plainBody(), QString("NEW"));
			CHECK(msg2.plainBody(), QString("foo"));
		}
		CHECK(msg1.plainBody(), QString("NEW"));

		msg1 = msg1;
		CHECK(msg1.plainBody(), QString("NEW"));
	}
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
				//kdDebug() << "checking known-broken testcase: " << fileName << endl;
				XFAIL(result, expectedData);
			}
			else
			{
				//kdDebug() << "checking known-working testcase: " << fileName << endl;
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

// vim: set noet ts=4 sts=4 sw=4:
