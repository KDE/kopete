/*
   mimeheader_test.cpp - Unittest for Papillon::MimeHeader

   Copyright (c) 2006 by Michaël Larouche <michael.larouche@kdemail.net>

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
#include "mimeheader.h"

void MimeHeader_Test::testMimeParsing()
{
	QString sourceData = QString( QLatin1String("MSG Hotmail Hotmail 425\r\nMIME-Version: 1.0\r\nContent-Type: text/x-msmsgsprofile\r\n\r\nBlhjajskldjaksjdkjrlwer") );
	QString expectedGeneratedMimeHeader = QString( QLatin1String("MIME-Version: 1.0\r\nContent-Type: text/x-msmsgsprofile\r\n") );

	Papillon::MimeHeader test = Papillon::MimeHeader::parseMimeHeader(sourceData);
	
	QCOMPARE( test.value( QLatin1String("MIME-Version") ).toString(), QString("1.0") );
	QCOMPARE( test.value( QLatin1String("Content-Type") ).toString(), QString("text/x-msmsgsprofile") );
	QCOMPARE( test.toString(), expectedGeneratedMimeHeader );
}

QTEST_MAIN(MimeHeader_Test)

#include "mimeheader_test.moc"
