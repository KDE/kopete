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

#include "kopetemessage_test.h"

// QTestLib for KDE
#include <qtest_kde.h>
#include <stdlib.h>

#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QByteArray>

#include <kstandarddirs.h>
#include <kcomponentdata.h>
#include <k3process.h>
#include <kdebug.h>

#include "kopetemessage_test.moc"
#include "kopeteaccount_mock.h"
#include "kopeteprotocol_mock.h"
#include "kopetecontact_mock.h"
#include "kopetemetacontact_mock.h"

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
	m_contactFrom = new Kopete::Test::Mock::Contact(m_account, QString::fromLatin1("test-myself"), m_metaContactMyself, QString());
	m_contactTo = new Kopete::Test::Mock::Contact(m_account, QString::fromLatin1("test-dest"), m_metaContactOther, QString());
}

void KopeteMessage_Test::testPrimitives()
{
	/**********************************************
	 * from(), to()
	 *********************************************/

	{
		Kopete::Message msg( m_contactFrom, m_contactTo);
		Q_ASSERT(msg.from());
		Q_ASSERT(!msg.to().isEmpty());
	}

	/**********************************************
	 * Direction
	 *********************************************/

	{
		Kopete::Message msg;
		msg.setDirection( Kopete::Message::Inbound );
		QCOMPARE(msg.direction(), Kopete::Message::Inbound);
	}
	{
		Kopete::Message msg;
		msg.setDirection( Kopete::Message::Outbound );
		QCOMPARE(msg.direction(), Kopete::Message::Outbound);
	}
	{
		Kopete::Message msg;
		msg.setDirection( Kopete::Message::Internal );
		QCOMPARE(msg.direction(), Kopete::Message::Internal);
	}

	/**********************************************
	 * Message Format
	 *********************************************/

	{
		Kopete::Message msg;
		msg.setPlainBody( QLatin1String("foobar") );
		QCOMPARE(msg.format(), Qt::PlainText);
	}
	{
		Kopete::Message msg;
		msg.setHtmlBody( QLatin1String("foobar") );
		QCOMPARE(msg.format(), Qt::RichText);
	}
	{
		QString m = "foobar";
		Kopete::Message msg;

		msg.setPlainBody(m);
		QCOMPARE(msg.format(), Qt::PlainText);

		msg.setHtmlBody(m);
		QCOMPARE(msg.format(), Qt::RichText);
	}


	/**********************************************
	 * setBody()
	 *********************************************/

	{
		QString m = "foobar";
		Kopete::Message msg;
		msg.setHtmlBody( m );

		msg.setPlainBody("NEW");
		QCOMPARE(QString("NEW"), msg.plainBody());

		msg.setPlainBody("NEW_NEW");
		QCOMPARE(msg.plainBody(), QString("NEW_NEW"));
	}
	{
		QString m = "foobar";
		Kopete::Message msg;
		msg.setPlainBody( m );

		msg.setPlainBody("NEW");
		QCOMPARE(msg.plainBody(), QString("NEW"));

		msg.setHtmlBody("NEW_NEW");
		QCOMPARE(msg.plainBody(), QString("NEW_NEW"));
	}
	{
		QString m = "<html><head></head><body foo=\"bar\">   <b>HELLO WORLD</b>   </body></html>";
		Kopete::Message msg;
		msg.setPlainBody( m );
		QCOMPARE(msg.plainBody(), m);

		msg.setPlainBody("<simple> SIMPLE");
		QCOMPARE(msg.plainBody(), QString("<simple> SIMPLE"));

		msg.setHtmlBody("<simple>SIMPLE</simple>");
		QCOMPARE(msg.plainBody(),  QString("SIMPLE") );

		QCOMPARE(Kopete::Message::unescape( QString( "<simple>SIMPLE</simple>" ) ), QString("SIMPLE") );
		QCOMPARE(Kopete::Message::unescape( QString( "Foo <img src=\"foo.png\" />" ) ), QString("Foo ") );
		QCOMPARE(Kopete::Message::unescape( QString( "Foo <img src=\"foo.png\" title=\"Bar\" />" ) ), QString("Foo Bar") );

		msg.setHtmlBody(m);

//		QCOMPARE(msg.escapedBody(),           QString(" &nbsp; <b>HELLO WORLD</b> &nbsp; "));
//		QCOMPARE(msg.plainBody(),             QString("   HELLO WORLD   "));
		QCOMPARE(msg.plainBody().trimmed(),   QString("HELLO WORLD"));
//		QCOMPARE(msg.escapedBody().trimmed(), QString("&nbsp; <b>HELLO WORLD</b> &nbsp;"));
	}

	/**********************************************
	 * Copy constructor
	 *********************************************/

	{
		Kopete::Message msg1;
		msg1.setHtmlBody( QLatin1String("foo") );
		Kopete::Message msg2(msg1);

		QCOMPARE(msg1.plainBody(), msg2.plainBody());
		QCOMPARE(msg1.escapedBody(), msg2.escapedBody());

		msg1.setPlainBody("NEW");
		QCOMPARE(msg1.plainBody(), QString("NEW"));
		QCOMPARE(msg2.plainBody(), QString("foo"));
	}

	/**********************************************
	 * operator=
	 *********************************************/

	{
		Kopete::Message msg1;
		msg1.setHtmlBody( QLatin1String("foo") );
		{
			Kopete::Message msg2;

//			QCOMPARE(msg2.plainBody(), QString());

			msg2 = msg1;

			QCOMPARE(msg1.plainBody(), msg2.plainBody());
			QCOMPARE(msg1.escapedBody(), msg2.escapedBody());

			msg1.setPlainBody("NEW");
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
	for ( QStringList::ConstIterator it = inputFileNames.constBegin(); it != inputFileNames.constEnd(); ++it)
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
			Qt::TextFormat format;
			if ( fileName.section('-', 1, 1) == QString::fromLatin1("plaintext") )
				format = Qt::PlainText;
			else
				format = Qt::RichText;
	
			QString result = Kopete::Message::parseLinks( inputData, format );

			// HACK to know the test case we applied, concatenate testcase name to both
			// input and expected string. WIll remove when I can add some sort of metadata
			// to a QCOMPARE so debug its origin testcase
			result = fileName + QString::fromLatin1(": ") + result;
			expectedData = fileName + QString::fromLatin1(": ") + expectedData;
			// if the test case begins with broken, we expect it to fail, then use XFAIL
			// otherwise use QCOMPARE
			if ( fileName.section('-', 0, 0) == QString::fromLatin1("broken") )
			{
				//kDebug() << "checking known-broken testcase: " << fileName;
				QEXPECT_FAIL("", "Checking know-broken testcase", Continue);
				QCOMPARE(result, expectedData);
			}
			else
			{
				//kDebug() << "checking known-working testcase: " << fileName;
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
