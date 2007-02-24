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

// QTestLib for KDE
#include <qtest_kde.h>
#include <stdlib.h>

#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QByteArray>

#include <kapplication.h>
#include <kstandarddirs.h>
#include <kcomponentdata.h>
#include <kprocess.h>
#include <kdebug.h>

#include "kopetemessage_test.h"
#include "kopetemessage_test.moc"
#include "kopeteaccount_mock.h"
#include "kopeteprotocol_mock.h"
#include "kopetecontact_mock.h"
#include "kopetemetacontact_mock.h"
#include "kopeteaccount_mock.h"

QTEST_KDEMAIN( KopeteMessage_Test, NoGUI )

/*
  There are four sets of tests: for each of plain text and html, we have those
  known to work in the current codebase, and those known to fail right now.

  the name convention is working|broken-plaintext|html-number.input|output
*/

KopeteMessage_Test::KopeteMessage_Test()
{
	// change user data dir to avoid messing with user's .kde dir
	setenv( "KDEHOME", QFile::encodeName( QDir::homePath() + "/.kopete-unittest" ), true );

	// create fake objects needed to build a reasonable testeable message
	m_protocol = new Kopete::Test::Mock::Protocol( KComponentData(QByteArray("test-kopete-message")), 0L);
	m_account = new Kopete::Test::Mock::Account(m_protocol, "testaccount");
	m_metaContactMyself = new Kopete::Test::Mock::MetaContact();
	m_metaContactOther = new Kopete::Test::Mock::MetaContact();
	m_contactFrom = new Kopete::Test::Mock::Contact(m_account, QString::fromLatin1("test-myself"), m_metaContactMyself, QString::null);
	m_contactTo = new Kopete::Test::Mock::Contact(m_account, QString::fromLatin1("test-dest"), m_metaContactOther, QString::null);
	m_message = new Kopete::Message( m_contactFrom, m_contactTo, QString::null, Kopete::Message::Outbound, Kopete::Message::PlainText);
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
		QCOMPARE(Kopete::Message::Inbound, msg.direction());
	}
	{
		Kopete::Message msg( m_contactFrom, m_contactTo, "foobar", Kopete::Message::Outbound, Kopete::Message::RichText);
		QCOMPARE(Kopete::Message::Outbound, msg.direction());
	}
	{
		Kopete::Message msg( m_contactFrom, m_contactTo, "foobar", Kopete::Message::Internal, Kopete::Message::RichText);
		QCOMPARE(Kopete::Message::Internal, msg.direction());
	}

	/**********************************************
	 * Message Format
	 *********************************************/

	{
		Kopete::Message msg( m_contactFrom, m_contactTo, "foobar", Kopete::Message::Inbound, Kopete::Message::PlainText);
		QCOMPARE(Kopete::Message::PlainText, msg.format());
	}
	{
		Kopete::Message msg( m_contactFrom, m_contactTo, "foobar", Kopete::Message::Inbound, Kopete::Message::RichText);
		QCOMPARE(Kopete::Message::RichText, msg.format());
	}
	{
		QString m = "foobar";
		Kopete::Message msg( m_contactFrom, m_contactTo, m, Kopete::Message::Inbound, Kopete::Message::RichText);

		msg.setBody(m, Kopete::Message::PlainText);
		QCOMPARE(Kopete::Message::PlainText, msg.format());

		msg.setBody(m, Kopete::Message::RichText);
		QCOMPARE(Kopete::Message::RichText, msg.format());

		msg.setBody(m, Kopete::Message::ParsedHTML);
		QCOMPARE(Kopete::Message::ParsedHTML, msg.format());

		msg.setBody(m, Kopete::Message::Crypted);
		QCOMPARE(Kopete::Message::Crypted, msg.format());
	}


	/**********************************************
	 * setBody()
	 *********************************************/

	{
		QString m = "foobar";
		Kopete::Message msg( m_contactFrom, m_contactTo, m, Kopete::Message::Inbound, Kopete::Message::RichText);

		msg.setBody("NEW", Kopete::Message::PlainText);
		QCOMPARE(QString("NEW"), msg.plainBody());

		msg.setBody("NEW_NEW", Kopete::Message::RichText);
		QCOMPARE(QString("NEW_NEW"), msg.plainBody());
	}
	{
		QString m = "foobar";
		Kopete::Message msg( m_contactFrom, m_contactTo, m, Kopete::Message::Inbound, Kopete::Message::PlainText);

		msg.setBody("NEW", Kopete::Message::PlainText);
		QCOMPARE(QString("NEW"), msg.plainBody());

		msg.setBody("NEW_NEW", Kopete::Message::RichText);
		QCOMPARE(QString("NEW_NEW"), msg.plainBody());
	}
	{
		QString m = "<html><head></head><body foo=\"bar\">   <b>HELLO WORLD</b>   </body></html>";
		Kopete::Message msg( m_contactFrom, m_contactTo, m, Kopete::Message::Inbound, Kopete::Message::PlainText);
		QCOMPARE(m, msg.plainBody());

		msg.setBody("<simple> SIMPLE", Kopete::Message::PlainText);
		QCOMPARE(QString("<simple> SIMPLE"), msg.plainBody());
		QCOMPARE(QString("&lt;simple&gt; SIMPLE"), msg.escapedBody());

		msg.setBody("<simple>SIMPLE</simple>", Kopete::Message::RichText);
		QCOMPARE(msg.plainBody(),   QString("SIMPLE") );
		QCOMPARE(msg.escapedBody(), QString("<simple>SIMPLE</simple>") );

		QCOMPARE(Kopete::Message::unescape( QString( "<simple>SIMPLE</simple>" ) ), QString("SIMPLE") );

		msg.setBody(m, Kopete::Message::RichText);

		// FIXME: Should setBody() also strip extra white space?
		//QCOMPARE(msg.plainBody(),   QString("HELLO WORLD"));
		//QCOMPARE(msg.escapedBody(), QString("<b>HELLO WORLD</b>"));

		QCOMPARE(msg.escapedBody(),                   QString(" &nbsp; <b>HELLO WORLD</b> &nbsp; "));
		QCOMPARE(msg.plainBody(),                     QString("   HELLO WORLD   "));
		QCOMPARE(msg.plainBody().stripWhiteSpace(),   QString("HELLO WORLD"));
		QCOMPARE(msg.escapedBody().stripWhiteSpace(), QString("&nbsp; <b>HELLO WORLD</b> &nbsp;"));
	}
	{
		Kopete::Message msg( m_contactFrom, m_contactTo, "foo", Kopete::Message::Inbound, Kopete::Message::PlainText);

		msg.setBody("<p>foo", Kopete::Message::RichText);
		QCOMPARE(msg.escapedBody(), QString("foo"));

		msg.setBody("<p>foo</p>", Kopete::Message::RichText);
		QCOMPARE(msg.escapedBody(), QString("foo"));

		msg.setBody("\n<p>foo</p>\n<br/>", Kopete::Message::RichText);
		QCOMPARE(msg.escapedBody(), QString("foo<br/>"));
	}

	/**********************************************
	 * Copy constructor
	 *********************************************/

	{
		Kopete::Message msg1(m_contactFrom, m_contactTo, "foo", Kopete::Message::Inbound, Kopete::Message::RichText);
		Kopete::Message msg2(msg1);

		QCOMPARE(msg1.plainBody(), msg2.plainBody());
		QCOMPARE(msg1.escapedBody(), msg2.escapedBody());

		msg1.setBody("NEW", Kopete::Message::PlainText);
		QCOMPARE(msg1.plainBody(), QString("NEW"));
		QCOMPARE(msg2.plainBody(), QString("foo"));
	}

	/**********************************************
	 * operator=
	 *********************************************/

	{
		Kopete::Message msg1(m_contactFrom, m_contactTo, "foo", Kopete::Message::Inbound, Kopete::Message::RichText);
		{
			Kopete::Message msg2;

//			CHECK(msg2.plainBody(), QString::null);

			msg2 = msg1;

			QCOMPARE(msg1.plainBody(), msg2.plainBody());
			QCOMPARE(msg1.escapedBody(), msg2.escapedBody());

			msg1.setBody("NEW", Kopete::Message::PlainText);
			QCOMPARE(msg1.plainBody(), QString("NEW"));
			QCOMPARE(msg2.plainBody(), QString("foo"));
		}
		QCOMPARE(msg1.plainBody(), QString("NEW"));

		msg1 = msg1;
		QCOMPARE(msg1.plainBody(), QString("NEW"));
	}
}

void KopeteMessage_Test::testLinkParser()
{
	QString basePath = QString::fromLatin1( SRCDIR ) + QString::fromLatin1("/link-parser-testcases");
	QDir testCasesDir(basePath);
	
	QStringList inputFileNames = testCasesDir.entryList(QStringList(QLatin1String("*.input")));
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
			QSKIP("Warning! expected output for testcase not found. Skiping testcase", SkipSingle);
			continue;
		}
		if ( inputFile.open( QIODevice::ReadOnly ) && expectedFile.open( QIODevice::ReadOnly ))
		{
			QTextStream inputStream(&inputFile);
			QTextStream expectedStream(&expectedFile);
			QString inputData;
			QString expectedData;
			inputData = inputStream.readAll();
			expectedData = expectedStream.readAll();

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
				//kDebug() << "checking known-broken testcase: " << fileName << endl;
				QEXPECT_FAIL("", "Checking know-broken testcase", Continue);
				QCOMPARE(result, expectedData);
			}
			else
			{
				//kDebug() << "checking known-working testcase: " << fileName << endl;
				QCOMPARE(result, expectedData);
			}
		}
		else
		{
			QSKIP("Warning! can't open testcase files for. Skiping testcase", SkipSingle);
			continue;
		}
	}
}

// vim: set noet ts=4 sts=4 sw=4:
