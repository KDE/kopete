/*
   mimeheader_test.cpp - Unittest for Papillon::MimeHeader

   Copyright (c) 2006 by Michaël Larouche <larouche@kde.org>

   *************************************************************************
   *                                                                       *
   * This library is free software; you can redistribute it and/or         *
   * modify it under the terms of the GNU Lesser General Public            *
   * License as published by the Free Software Foundation; either          *
   * version 2 of the License, or (at your option) any later version.      *
   *                                                                       *
   *************************************************************************
*/
// Qt includes
#include "mimeheader_test.h"

#include <QtTest/QtTest>
#include <QtCore/QLatin1String>

// Papillon includes
#include "Papillon/MimeHeader"

using namespace Papillon;

void MimeHeader_Test::testMimeParsing()
{
	QString sourceData = QLatin1String("MSG Hotmail Hotmail 425\r\nMIME-Version: 1.0\r\nContent-Type: text/x-msmsgsprofile; charset=UTF-8\r\n\r\nBlhjajskldjaksjdkjrlwer");
	
	MimeHeader test = MimeHeader::parseMimeHeader(sourceData);
	
	QCOMPARE( test.value( QLatin1String("MIME-Version") ).toString(), QString("1.0") );
	QCOMPARE( test.contentType(), QString("text/x-msmsgsprofile") );
	QCOMPARE( test.charset() , QString("UTF-8") );
}

void MimeHeader_Test::testMimeGeneration()
{
	QString expectedGeneratedMimeHeader = QLatin1String("MIME-Version: 1.0\r\nContent-Type: text/plain; charset=UTF-8\r\nTest: Kopete\r\nFrom: test@papillon\r\n");

	MimeHeader test;
	test.setMimeVersion(); // Set 1.0 by default
	test.setValue( QLatin1String("Test"), QLatin1String("Kopete") );
	test.setContentType( QLatin1String("text/plain") );
	test.setValue( QLatin1String("From"), QLatin1String("test@papillon") );
	test.setCharset( QLatin1String("UTF-8") );

	QCOMPARE( test.toString(), expectedGeneratedMimeHeader );
}

QTEST_MAIN(MimeHeader_Test)

#include "mimeheader_test.moc"
