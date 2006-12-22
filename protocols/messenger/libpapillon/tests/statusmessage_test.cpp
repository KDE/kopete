/*
   statusmessage_test.h - Unittest for Papillon::StatusMessage

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
#include <QtTest/QtTest>
#include <QtCore/QLatin1String>

// Papillon includes
#include "Papillon/StatusMessage"

#include "statusmessage_test.h"

using namespace Papillon;

void StatusMessage_Test::testStatusMessageParsing()
{
	QString testXmlString("<Data><PSM>Papillon::StatusMessage&apos;s test &lt;&gt;</PSM><CurrentMedia>\\0Music\\01\\0{0} - {1}\\0Song Title\\0Song Artist\\0Song Album\\0\\0</CurrentMedia></Data>");

	StatusMessage statusMessage = StatusMessage::fromXml(testXmlString);

	QCOMPARE( statusMessage.message(), QString("Papillon::StatusMessage's test <>") );
	
	QVERIFY( statusMessage.currentMediaApplication().isEmpty() );
	QCOMPARE( static_cast<int>(statusMessage.currentMediaType()), static_cast<int>(Presence::MediaMusic) );
	QVERIFY( statusMessage.isCurrentMediaEnabled() );
	QCOMPARE( statusMessage.currentMediaFormatterString(), QString("{0} - {1}") );
	QCOMPARE( statusMessage.formattedMediaString(), QString("Song Title - Song Artist") );
}

void StatusMessage_Test::testStatusMessageGeneration()
{
	QString expectedXmlString("<Data><PSM>Papillon::StatusMessage's test &lt;> test</PSM><CurrentMedia>Amarok\\0Music\\01\\0{0} - {1} ({2})\\0High Hopes\\0Pink Floyd\\0Division Bell\\0</CurrentMedia></Data>");

	StatusMessage statusMessage("Papillon::StatusMessage's test <> test");
	
	statusMessage.setCurrentMediaApplication( QLatin1String("Amarok") );
	statusMessage.setCurrentMediaType( Presence::MediaMusic );
	statusMessage.setCurrentMediaFormatterString( QLatin1String("{0} - {1} ({2})") );
	
	QList<QVariant> args;
	args << QString("High Hopes");
	args << QString("Pink Floyd");
	args << QString("Division Bell");
	statusMessage.setCurrentMediaArguments(args);

	QCOMPARE( statusMessage.toXml(), expectedXmlString );
}

QTEST_MAIN(StatusMessage_Test)

#include "statusmessage_test.moc"
